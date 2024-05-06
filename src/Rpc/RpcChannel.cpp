#include "RpcChannel.h"
#include "Log/Log.h"
#include "Rpc/PBProtocol.h"
#include "Rpc/RpcController.h"
#include "Tcp/TcpClient.h"
#include "ThreadPool/ThreadPool.h"
#include "google/protobuf/message.h" // Add this include statement
#include "google/protobuf/service.h"

namespace MyTinyRPC
{
    RpcChannel::RpcChannel()
    {
    }

    RpcChannel::~RpcChannel()
    {
    }

    void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                                google::protobuf::RpcController *controller,
                                const google::protobuf::Message *request,
                                google::protobuf::Message *response,
                                google::protobuf::Closure *done)
    {
        if (!m_flag_init)
        {
            LOG_ERROR("m_flag_init is false");
            return;
        }
        if (!controller || !request || !response)
        {
            LOG_ERROR("controller or request or response or done is null");
            return;
        }
        RpcController *rpcController = dynamic_cast<RpcController *>(controller);
        PBProtocol::s_ptr pbProtocol = std::make_shared<PBProtocol>();
        std::string msg_id = rpcController->getId();
        if (msg_id.empty())
        {
            LOG_ERROR("msg_id is empty");
            rpcController->SetFailed("msg_id is empty");
            return;
        }
        pbProtocol->m_msg_id = msg_id;
        pbProtocol->m_msg_err = "";
        pbProtocol->m_msg_method_name = method->full_name();
        request->SerializeToString(&pbProtocol->m_pb_data);
        TcpBuffer::s_ptr tcpBuffer = pbProtocol->encode();
        Status status =
            TcpClient::getInstance()->Send(tcpBuffer->getBuffer(), tcpBuffer->readAbleSize());
        if (status)
        {
            LOG_INFO("%s", status.getMessage().c_str());
            status = TcpClient::getInstance()->addWait(pbProtocol->m_msg_id, shared_from_this());
            if (status)
            {
                LOG_INFO("%s", status.getMessage().c_str());
            }
            else
            {
                LOG_ERROR("%s", status.getMessage().c_str());
            }
        }
        else
        {
            LOG_ERROR("%s", status.getMessage().c_str());
        }
    }

    Status RpcChannel::callback()
    {
        if (m_closure != nullptr)
        {
            m_closure->Run();
            return Status(Status::OK, "callback success");
        }
        else
        {
            return Status(Status::ERROR, "callback method is null");
        }
    }

    void RpcChannel::init(ppcController_s_ptr controller,
                          message_s_ptr request,
                          message_s_ptr response,
                          rpcClosure_s_ptr closure)
    {
        m_controller = controller;
        m_request = request;
        m_response = response;
        m_closure = closure;
        m_flag_init = true;
    }

    RpcChannel::ppcController_s_ptr RpcChannel::getController()
    {
        return m_controller;
    }

    RpcChannel::message_s_ptr RpcChannel::getRequest()
    {
        return m_request;
    }

    RpcChannel::message_s_ptr RpcChannel::getResponse()
    {
        return m_response;
    }

    RpcChannel::rpcClosure_s_ptr RpcChannel::getClosure()
    {
        return m_closure;
    }

} // namespace MyTinyRPC
