#include "RpcDispatcher.h"
#include "Log/Log.h"

namespace MyTinyRPC
{
    static RpcDispatcher::s_ptr global_rpc_dispatcher = nullptr;

    RpcDispatcher::s_ptr RpcDispatcher::getInstance()
    {
        if (global_rpc_dispatcher == nullptr)
        {
            global_rpc_dispatcher = std::make_shared<RpcDispatcher>();
        }
        return global_rpc_dispatcher;
    }

    void RpcDispatcher::registerService(service_s_ptr service)
    {
        m_serviceMap[service->GetDescriptor()->full_name()] = service;
    }

    RpcDispatcher::service_s_ptr RpcDispatcher::getService(const std::string &name)
    {
        return m_serviceMap[name];
    }

    std::string RpcDispatcher::getServiceName(const std::string &full_name)
    {
        size_t i = full_name.find_first_of(".");
        if (i == full_name.npos)
        {
            LOG_ERROR("not find . in full name [%s]", full_name.c_str());
        }
        return full_name.substr(0, i);
    }

    std::string RpcDispatcher::getMethodName(const std::string &full_name)
    {
        size_t i = full_name.find_first_of(".");
        if (i == full_name.npos)
        {
            LOG_ERROR("not find . in full name [%s]", full_name.c_str());
        }
        return full_name.substr(i + 1, full_name.length() - i - 1);
    }

    TcpBuffer::s_ptr RpcDispatcher::dispatcher(PBProtocol::s_ptr protocol)
    {
        RpcDispatcher::service_s_ptr service = RpcDispatcher::getInstance()->getService(
            RpcDispatcher::getInstance()->getServiceName(protocol->m_msg_method_name));
        if (service == nullptr)
        {
            throw std::runtime_error("Fatal: Service not found");
        }
        const google::protobuf::MethodDescriptor *method =
            service->GetDescriptor()->FindMethodByName(
                RpcDispatcher::getInstance()->getMethodName(protocol->m_msg_method_name));
        if (method == nullptr)
        {
            throw std::runtime_error("Fatal: Method not found");
        }
        google::protobuf::Message *req = service->GetRequestPrototype(method).New();
        if (!req->ParseFromString(protocol->m_pb_data))
        {
            throw std::runtime_error("Fatal: Error parsing request");
        }
        google::protobuf::Message *res = service->GetResponsePrototype(method).New();

        service->CallMethod(method, nullptr, req, res, nullptr);

        PBProtocol::s_ptr pbProtocol = std::make_shared<PBProtocol>();
        pbProtocol->m_msg_id = protocol->m_msg_id;
        pbProtocol->m_msg_err = "";
        pbProtocol->m_msg_method_name = protocol->m_msg_method_name;
        res->SerializeToString(&pbProtocol->m_pb_data);
        TcpBuffer::s_ptr retBuffer = pbProtocol->encode();
        return retBuffer;
    }

    void RpcDispatcher::callMethod(const std::string &full_name,
                                   google::protobuf::RpcController *controller,
                                   const google::protobuf::Message *request,
                                   google::protobuf::Message *response,
                                   google::protobuf::Closure *done)
    {
        std::string service_name = getServiceName(full_name);
        std::string method_name = getMethodName(full_name);
        service_s_ptr service = getService(service_name);
        const google::protobuf::MethodDescriptor *method =
            service->GetDescriptor()->FindMethodByName(method_name);
        service->CallMethod(method, controller, request, response, done);
    }

} // namespace MyTinyRPC
