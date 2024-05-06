#include "TcpServer.h"
#include "Log/Log.h"
#include "Rpc/PBProtocol.h"
#include "Rpc/RpcChannel.h"
#include "Rpc/RpcDispatcher.h"
#include "Tcp/TcpBuffer.h"
#include "ThreadPool/ThreadPool.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <google/protobuf/service.h>
#include <memory>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace MyTinyRPC
{
    TcpServer::TcpServer(int port)
    {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0)
        {
            LOG_ERROR("Error creating server socket");
            return;
        }

        memset(&localAddr, 0, sizeof(localAddr));
        localAddr.sin_family = AF_INET;
        localAddr.sin_port = htons(port);
        localAddr.sin_addr.s_addr = htonl(INADDR_ANY);

        // 设置SO_REUSEADDR选项以允许端口重用
        int opt = 1;
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        {
            throw std::runtime_error("Fatal: setting socket options failed");
        }

        if (bind(serverSocket, (struct sockaddr *)&localAddr, sizeof(localAddr)) < 0)
        {
            LOG_ERROR("Error binding server socket");
            return;
        }

        if (listen(serverSocket, 5) < 0)
        {
            LOG_ERROR("Error listening on server socket");
            return;
        }

        ThreadPool::getInstance().enqueue(
            [&]()
            {
                while (true)
                {
                    {
                        std::unique_lock<std::mutex> lock_client_list(m_mutex_client_list);
                        // 处理m_client_list的所有请求
                        PBProtocol::s_ptr protocol = std::make_shared<PBProtocol>();
                        for (auto it = m_client_list.begin(); it != m_client_list.end(); it++)
                        {
                            while (true)
                            {
                                Status status = protocol->decode(it->second);
                                if (!status)
                                {

                                    if (status.getCode() == MyTinyRPC::Status::Code::CANCELLED)
                                    {
                                        LOG_INFO("%s", status.getMessage().c_str());
                                    }
                                    else
                                    {

                                        LOG_ERROR("%s", status.getMessage().c_str());
                                    }
                                    break;
                                }
                                else
                                {
                                    LOG_INFO("%s", status.getMessage().c_str());
                                }
                                TcpBuffer::s_ptr buffer =
                                    RpcDispatcher::getInstance()->dispatcher(protocol);
                                {
                                    std::unique_lock<std::mutex> lock_send_list(m_mutex_send_list);
                                    m_send_list.push(std::make_pair(
                                        it->first,
                                        std::make_pair(buffer, buffer->readAbleSize())));
                                }
                            }
                        }
                    }
                }
            });

        while (true)
        {
            int clientSocket =
                accept(serverSocket, (struct sockaddr *)&remoteAddr, (socklen_t *)&remoteAddr);

            if (clientSocket < 0)
            {
                throw std::runtime_error("Fatal: accepting client connection");
            }

            {
                std::unique_lock<std::mutex> lock_client_list(m_mutex_client_list);
                m_client_list[clientSocket] = std::make_shared<TcpBuffer>(1024 * 50);
            }
            // LOG_DEBUG("Client connected: %d", clientSocket);
            ThreadPool::getInstance().enqueue(
                [this, clientSocket]()
                {
                    int flags = fcntl(clientSocket, F_GETFL, 0);
                    if (flags == -1)
                    {
                        LOG_ERROR("Error getting socket flags");
                        exit(1);
                    }
                    fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK);

                    handleClient(clientSocket);
                });
        }
    }

    TcpServer::~TcpServer()
    {
        close(serverSocket);
    }

    void TcpServer::handleClient(int clientSocket)
    {
        while (true)
        {
            char buffer[bufferSize] = { 0 };
            int bytesRead = recv(clientSocket, buffer, bufferSize, 0);
            if (bytesRead <= 0)
            {
                if (bytesRead == 0)
                {
                    // 客户端已经关闭连接
                    LOG_INFO("Client has disconnected");
                    // 关闭客户端套接字
                    {
                        std::unique_lock<std::mutex> lock_client_list(m_mutex_client_list);
                        m_client_list.erase(clientSocket);
                    }
                    // 清空一个queue
                    {
                        std::unique_lock<std::mutex> lock_send_list(m_mutex_send_list);
                        while (!m_send_list.empty())
                        {
                            m_send_list.pop();
                        }
                    }
                }
                else if (bytesRead == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
                {
                    // 接收数据时出现错误
                    LOG_ERROR("Error reading from client socket: %s", strerror(errno));
                }
                else if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    std::unique_lock<std::mutex> lock_send_list(m_mutex_send_list);
                    while (!m_send_list.empty())
                    {
                        int socket = m_send_list.front().first;
                        TcpBuffer::s_ptr data = m_send_list.front().second.first;
                        int len = m_send_list.front().second.second;
                        m_send_list.pop();

                        // test1
                        if (send(socket, data->getBuffer(), len, 0) < 0)
                        {
                            LOG_ERROR("Error sending response to client");
                            closeClient(socket);
                        }

                        // test2
                        // PBProtocol::s_ptr protocol = std::make_shared<PBProtocol>();
                        // protocol->decode(data);
                        // if (protocol->m_msg_id != std::to_string(index++))
                        // {
                        //     std::cout << "get id " << protocol->m_msg_id << std::endl;
                        // }
                    }
                    continue;
                }
                break; // 退出循环，关闭连接
            }

            {
                std::unique_lock<std::mutex> lock_client_list(m_mutex_client_list);
                if (m_client_list.find(clientSocket) == m_client_list.end())
                {
                    LOG_ERROR("client socket has been closed");
                    return;
                }
                m_client_list[clientSocket]->writeFromChar(buffer, bytesRead);

                // test: 连接测试
                // PBProtocol::s_ptr protocol = std::make_shared<PBProtocol>();
                // while (protocol->decode(m_client_list[clientSocket]))
                // {
                //     std::cout << "get id " << protocol->m_msg_id << std::endl;
                // }
            }
            // m_condVar_client_list.notify_all();
        }
    }

    void TcpServer::closeClient(int clientSocket)
    {
        close(clientSocket);
    }

    static TcpServer::s_ptr global_tcp_server = nullptr;

    TcpServer::s_ptr TcpServer::getInstance(int port)
    {
        if (global_tcp_server == nullptr)
        {
            global_tcp_server = std::make_shared<TcpServer>(port);
        }
        return global_tcp_server;
    }
} // namespace MyTinyRPC