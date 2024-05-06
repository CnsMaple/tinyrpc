#include "Status.h"

MyTinyRPC::Status::Status(Code code, const std::string &message) : m_code(code), m_message(message)
{
}

MyTinyRPC::Status::Code MyTinyRPC::Status::getCode() const
{
    return m_code;
}

const std::string &MyTinyRPC::Status::getMessage() const
{
    return m_message;
}

bool MyTinyRPC::Status::isOk() const
{
    return m_code == OK;
}
