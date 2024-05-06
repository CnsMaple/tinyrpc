#ifndef PBPROTOCOL_H
#define PBPROTOCOL_H

#include "Tcp/TcpBuffer.h"
#include <memory>
#include <string>
#include "Status/Status.h"

namespace MyTinyRPC
{
    class PBProtocol
    {
        public:
            typedef std::shared_ptr<PBProtocol> s_ptr;
            MyTinyRPC::TcpBuffer::s_ptr encode();
            Status decode(MyTinyRPC::TcpBuffer::s_ptr &buffer);

            std::string m_msg_id;
            std::string m_msg_err;
            std::string m_msg_method_name;
            std::string m_pb_data;

            char const static FLAGSTART = 0x02;
            char const static FLAGEND = 0x07;
    };
} // namespace MyTinyRPC

#endif // PBPROTOCOL_H