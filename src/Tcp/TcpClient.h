#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "Rpc/RpcChannel.h"
#include "Status/Status.h"
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>

namespace MyTinyRPC
{
    class TcpClient : public std::enable_shared_from_this<TcpClient>
    {
        public:
            typedef std::shared_ptr<TcpClient> s_ptr;
            TcpClient();
            ~TcpClient();

            Status Connect(const char *ip, int port);
            Status Send(const char *data, int len);
            void Recv();
            void Close();

            Status addWait(std::string id, RpcChannel::s_ptr channel);

            static s_ptr getInstance();

        private:
            int m_sockfd;

            std::condition_variable m_condVar_con_clo;
            std::mutex m_mutex_con_clo;
            bool m_connected = false;
            bool m_closed = true;

            std::condition_variable m_condVar_wait_list;
            std::mutex m_mutex_wait_list;
            std::map<std::string, RpcChannel::s_ptr> m_wait_list;

            const static int bufferSize =
                1024 * 10; // 64k，这个是tcp的默认缓冲区大小，如果要更大，则需要窗口缩放
    };
} // namespace MyTinyRPC

#endif