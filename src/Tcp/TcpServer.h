#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "Tcp/TcpBuffer.h"
#include <arpa/inet.h>
#include <condition_variable>
#include <map>
#include <memory>
#include <queue>
#include <string>

namespace MyTinyRPC
{
    class TcpServer : public std::enable_shared_from_this<TcpServer>
    {
        public:
            typedef std::shared_ptr<TcpServer> s_ptr;
            TcpServer(int port);
            ~TcpServer();

            static s_ptr getInstance(int port = 8080);

        private:
            int serverSocket;

            std::mutex m_mutex_send_list;
            std::condition_variable m_condVar_send_list;
            std::queue<std::pair<int, std::pair<TcpBuffer::s_ptr, int>>> m_send_list;

            struct sockaddr_in localAddr, remoteAddr;

            void handleClient(int clientSocket);

            void closeClient(int clientSocket);

            std::mutex m_mutex_client_list;
            std::condition_variable m_condVar_client_list;
            std::map<int, TcpBuffer::s_ptr> m_client_list;

            int bufferSize = 1024 * 10; // 64k，这个是tcp的默认缓冲区大小，如果要更大，则需要窗口缩放
    };
} // namespace MyTinyRPC

#endif // TCP_SERVER_H