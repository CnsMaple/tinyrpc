#include "RpcClosure.h"
#include "Log/Log.h"
#include "Rpc/RpcController.h"

namespace MyTinyRPC
{
    RpcClosure::RpcClosure(std::function<void()> func) : m_func(func)
    {
    }

    RpcClosure::~RpcClosure()
    {
    }

    void RpcClosure::Run()
    {
        try
        {
            if (m_func != nullptr)
            {
                m_func();
                LOG_INFO("RpcClosure::Run success");
            }
            else
            {
                LOG_ERROR("RpcClosure::Run m_func is null");
            }
        }
        catch (...)
        {
            LOG_ERROR("RpcClosure::Run error");
        }
    }

} // namespace MyTinyRPC
