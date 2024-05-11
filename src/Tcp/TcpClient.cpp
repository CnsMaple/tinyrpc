#include "TcpClient.h"
#include "Log/Log.h"
#include "Rpc/PBProtocol.h"
#include "TcpBuffer.h"
#include "ThreadPool/ThreadPool.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <google/protobuf/message.h>
#include <memory>
#include <netinet/tcp.h>
#include <string.h>
#include <unistd.h>

namespace MyTinyRPC
{
    TcpClient::TcpClient()
    {
        m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (m_sockfd < 0)
        {
            LOG_ERROR("Error creating socket");
            return;
        }
    }

    TcpClient::~TcpClient()
    {
        Close();
    }

    Status TcpClient::Connect(const char *ip, int port)
    {
        {
            std::unique_lock<std::mutex> lock_con_clo(m_mutex_con_clo);
            if (m_connected)
            {
                return Status(Status::Code::ERROR, "Already connected");
            }
        }
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = inet_addr(ip);

        if (connect(m_sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
        {
            return Status(Status::Code::ERROR, "Error connecting to server");
        }
        {
            std::unique_lock<std::mutex> lock_con_clo(m_mutex_con_clo);
            m_connected = true;
            m_closed = false;
        }

        // // 设置TCP接收缓冲区大小
        // int recv_buf_size = 1024; // 例如，设置为1MB
        // if (setsockopt(m_sockfd, SOL_SOCKET, SO_RCVBUF, &recv_buf_size, sizeof(recv_buf_size)) <
        // 0)
        // {
        //     // std::cerr << "Error setting receive buffer size" << std::endl;
        //     close(m_sockfd);
        //     return Status(Status::Code::ERROR, "Error setting receive buffer size");
        // }

        // // 设置TCP发送缓冲区大小
        // int send_buf_size = 1024; // 例如，设置为1MB
        // if (setsockopt(m_sockfd, SOL_SOCKET, SO_SNDBUF, &send_buf_size, sizeof(send_buf_size)) <
        // 0)
        // {
        //     // std::cerr << "Error setting send buffer size" << std::endl;
        //     close(m_sockfd);
        //     return Status(Status::Code::ERROR, "Error setting send buffer size");
        // }
        // socklen_t optlen = sizeof(int);
        // int recv_buf_size, send_buf_size;

        // // 获取TCP接收缓冲区的大小
        // if (getsockopt(m_sockfd, SOL_SOCKET, SO_RCVBUF, &recv_buf_size, &optlen) < 0)
        // {
        //     std::cerr << "Error getting receive buffer size" << std::endl;
        //     close(m_sockfd);
        // }

        // // 获取TCP发送缓冲区的大小
        // if (getsockopt(m_sockfd, SOL_SOCKET, SO_SNDBUF, &send_buf_size, &optlen) < 0)
        // {
        //     std::cerr << "Error getting send buffer size" << std::endl;
        //     close(m_sockfd);
        // }

        // std::cout << "Default receive buffer size: " << recv_buf_size << std::endl;
        // std::cout << "Default send buffer size: " << send_buf_size << std::endl;

        // 设置套接字为非阻塞模式，要在连接之后设置
        // int flags = fcntl(m_sockfd, F_GETFL, 0);
        // if (flags == -1)
        // {
        //     LOG_ERROR("Error getting socket flags");
        //     exit(1);
        // }
        // fcntl(m_sockfd, F_SETFL, flags | O_NONBLOCK);

        ThreadPool::getInstance().enqueue(
            [&]()
            {
                Recv();
            });
        return Status(Status::Code::OK, "Connected");
    }

    Status TcpClient::Send(const char *data, int len)
    {
        {
            std::unique_lock<std::mutex> lock_con_clo(m_mutex_con_clo);
            if (!m_connected)
            {
                return Status(Status::Code::ERROR, "Not connected");
            }
        }
        if (send(m_sockfd, data, len, 0) < 0)
        {
            return Status(Status::Code::ERROR,
                          formatString("Error sending data, reason: %s", strerror(errno)));
        }
        return Status(Status::Code::OK, "Sent data successfully");
    }

    void TcpClient::Recv()
    {
        TcpBuffer::s_ptr tcpBuffer = std::make_shared<TcpBuffer>(1024 * 50);
        while (true)
        {
            // 减少锁的范围
            {
                // todo: 可以做超时功能
                std::unique_lock<std::mutex> lock_con_clo(m_mutex_con_clo);
                if (m_closed && !m_connected)
                {
                    // 关了
                    std::unique_lock<std::mutex> lock_wait_list(m_mutex_wait_list);
                    if (m_wait_list.size() == 0)
                    {
                        LOG_INFO("Connection closed");
                        close(m_sockfd);
                        break;
                    }
                }
                else if (!m_closed && m_connected)
                {
                    // 没关
                    std::unique_lock<std::mutex> lock_wait_list(m_mutex_wait_list);
                    if (m_wait_list.size() == 0)
                    {
                        continue;
                    }
                }
                else
                {
                    throw std::runtime_error("Fatal: m_closed and m_connected both value");
                }
            }

            // 在这里释放锁，进行recv操作
            char data[bufferSize];
            int result = recv(m_sockfd, data, bufferSize, 0);
            if (result == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
            {
                LOG_ERROR("Error receiving data");
            }
            else if (result > 0)
            {
                tcpBuffer->writeFromChar(data, result);
                PBProtocol::s_ptr protocol = std::make_shared<PBProtocol>();
                // 再次获取锁，处理等待列表
                while (Status status = protocol->decode(tcpBuffer))
                {
                    LOG_INFO("%s", status.getMessage().c_str());
                    RpcChannel::s_ptr rpc_it = nullptr;
                    bool is_exist = false;
                    {
                        std::unique_lock<std::mutex> lock_wait_list(m_mutex_wait_list);
                        auto it = m_wait_list.find(protocol->m_msg_id);
                        if (it != m_wait_list.end())
                        {
                            rpc_it = it->second;
                            is_exist = true;
                            m_wait_list.erase(it);
                        }
                    }
                    if (is_exist)
                    {
                        if (rpc_it)
                        {
                            rpc_it->getResponse()->ParseFromString(protocol->m_pb_data);
                            rpc_it->callback();
                        }
                    }
                    else
                    {
                        LOG_ERROR("recv error,unknown id:%s", protocol->m_msg_id.c_str());
                    }
                }
            }
            // 如果recv返回0，表示对方已经关闭连接，可以根据需要进行处理
            else if (result == 0)
            {
                LOG_ERROR("Server closed the connection");
                {
                    std::unique_lock<std::mutex> lock_con_clo(m_mutex_con_clo);
                    m_closed = true;
                    m_connected = false;
                }
                {
                    std::unique_lock<std::mutex> lock_wait_list(m_mutex_wait_list);
                    m_wait_list.clear();
                }
            }
            else
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            m_condVar_wait_list.notify_all();
        }
    }

    void TcpClient::Close()
    {
        {
            std::unique_lock<std::mutex> lock_con_clo(m_mutex_con_clo);
            if (m_closed)
            {
                m_connected = false;
                return;
            }
        }
        {
            std::unique_lock<std::mutex> lock_wait_list(m_mutex_wait_list);
            m_condVar_wait_list.wait(lock_wait_list,
                                     [&]()
                                     {
                                         return m_wait_list.size() == 0;
                                     });
        }
        {
            std::unique_lock<std::mutex> lock_con_clo(m_mutex_con_clo);
            m_closed = true;
            m_connected = false;
        }
    }

    Status TcpClient::addWait(std::string id, RpcChannel::s_ptr channel)
    {
        {
            std::unique_lock<std::mutex> lock_con_clo(m_mutex_con_clo);
            if (m_closed)
            {
                return Status(Status::Code::ERROR, "Connection closed");
            }
        }
        {
            std::unique_lock<std::mutex> lock_wait_list(m_mutex_wait_list);
            if (m_wait_list.find(id) != m_wait_list.end())
            {
                return Status(Status::Code::ERROR, "id already exists");
            }
            m_wait_list[id] = channel;
        }
        return Status(Status::Code::OK, "addWait success");
    }

    static TcpClient::s_ptr global_tcp_client = nullptr;

    TcpClient::s_ptr TcpClient::getInstance()
    {
        if (global_tcp_client == nullptr)
        {
            global_tcp_client = std::make_shared<TcpClient>();
        }
        return global_tcp_client;
    }
} // namespace MyTinyRPC
