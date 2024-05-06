#ifndef RPC_DISPATCHER_H
#define RPC_DISPATCHER_H

#include "Rpc/PBProtocol.h"
#include "Tcp/TcpBuffer.h"
#include "memory"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/service.h>

namespace MyTinyRPC
{
    class RpcDispatcher
    {
        public:
            typedef std::shared_ptr<google::protobuf::Service> service_s_ptr;
            typedef std::shared_ptr<RpcDispatcher> s_ptr;
            static s_ptr getInstance();

            void registerService(service_s_ptr service);

            service_s_ptr getService(const std::string &name);

            std::string getServiceName(const std::string &full_name);

            std::string getMethodName(const std::string &full_name);

            TcpBuffer::s_ptr dispatcher(PBProtocol::s_ptr protocol);

            void callMethod(const std::string &full_name,
                            google::protobuf::RpcController *controller,
                            const google::protobuf::Message *request,
                            google::protobuf::Message *response,
                            google::protobuf::Closure *done);

        private:
            std::map<std::string, service_s_ptr> m_serviceMap = {};
    };
} // namespace MyTinyRPC

#endif // RPC_DISPATCHER_H