#include "RpcController.h"

namespace MyTinyRPC
{
    RpcController::RpcController()
    {
    }

    RpcController::~RpcController()
    {
    }

    void RpcController::Reset()
    {
        m_flag_failed = false;
        m_failed_reason = "";
        m_flag_canceled = false;
        m_id = "";
    }

    bool RpcController::Failed() const
    {
        return m_flag_failed;
    }

    std::string RpcController::ErrorText() const
    {
        if (m_flag_failed)
        {
            return m_failed_reason;
        }
        else
        {
            return "";
        }
    }

    void RpcController::StartCancel()
    {
        m_flag_canceled = true;
    }

    void RpcController::SetFailed(const std::string &reason)
    {
        m_flag_failed = true;
        m_failed_reason = reason;
    }

    bool RpcController::IsCanceled() const
    {
        return m_flag_canceled;
    }

    void RpcController::NotifyOnCancel(google::protobuf::Closure *callback)
    {
        if (m_flag_canceled && callback)
        {
            callback->Run();
        }
    }

    void RpcController::setId(std::string id)
    {
        m_id = id;
    }

    std::string RpcController::getId()
    {
        return m_id;
    }

} // namespace MyTinyRPC
