#ifndef RPCCHANNEL_H
#define RPCCHANNEL_H

#include "Rpc/RpcClosure.h"
#include "Rpc/RpcController.h"
#include <google/protobuf/service.h>
#include "Status/Status.h"

namespace MyTinyRPC
{

    // 继承了google::protobuf::RpcChannel需要实现CallMethod
    class RpcChannel : public google::protobuf::RpcChannel,
                       public std::enable_shared_from_this<RpcChannel>
    {
        public:
            RpcChannel();
            ~RpcChannel();
            typedef std::shared_ptr<RpcChannel> s_ptr;
            typedef std::shared_ptr<RpcController> ppcController_s_ptr;
            typedef std::shared_ptr<google::protobuf::Message> message_s_ptr;
            typedef std::shared_ptr<RpcClosure> rpcClosure_s_ptr;

            void CallMethod(const google::protobuf::MethodDescriptor *method,
                            google::protobuf::RpcController *controller,
                            const google::protobuf::Message *request,
                            google::protobuf::Message *response,
                            google::protobuf::Closure *done);
            Status callback();
            void init(ppcController_s_ptr controller,
                      message_s_ptr request,
                      message_s_ptr response,
                      rpcClosure_s_ptr closure);

            ppcController_s_ptr getController();
            message_s_ptr getRequest();
            message_s_ptr getResponse();
            rpcClosure_s_ptr getClosure();

        private:
            ppcController_s_ptr m_controller = nullptr;
            message_s_ptr m_request = nullptr;
            message_s_ptr m_response = nullptr;
            rpcClosure_s_ptr m_closure = nullptr;

            bool m_flag_init = false;
    };

} // namespace MyTinyRPC

#endif // RPCCHANNEL_H