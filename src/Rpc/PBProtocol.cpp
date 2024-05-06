#include "PBProtocol.h"
#include "Log/Log.h"
#include <arpa/inet.h>
#include <string.h>

#include <memory> // 引入shared_ptr的头文件

namespace MyTinyRPC
{
    TcpBuffer::s_ptr PBProtocol::encode()
    {
        // 基本数据大小
        int protocol_len =
            m_msg_id.size() + m_msg_err.size() + m_pb_data.size() + m_msg_method_name.size();
        // 两个标志位的大小
        protocol_len += 2;
        // 5个int32_t的大小，每个int32_t占4个字节(8bit/32=4)
        protocol_len += 5 * 4;
        std::shared_ptr<char> data(new char[protocol_len],
                                   std::default_delete<char[]>()); // 使用shared_ptr管理内存

        data.get()[0] = FLAGSTART;
        data.get()[protocol_len - 1] = FLAGEND;

        char *temp = data.get() + 1;

        int32_t protocol_len_len = htonl(protocol_len);
        memcpy(temp, &protocol_len_len, sizeof(protocol_len_len));
        temp += sizeof(protocol_len_len);

        int m_msg_id_len = m_msg_id.size();
        int32_t m_msg_id_len_len = htonl(m_msg_id_len);
        memcpy(temp, &m_msg_id_len_len, sizeof(m_msg_id_len_len));
        temp += sizeof(m_msg_id_len_len);
        memcpy(temp, m_msg_id.c_str(), m_msg_id_len);
        temp += m_msg_id_len;

        int m_msg_err_len = m_msg_err.size();
        int32_t m_msg_err_len_len = htonl(m_msg_err_len);
        memcpy(temp, &m_msg_err_len_len, sizeof(m_msg_err_len_len));
        temp += sizeof(m_msg_err_len_len);
        memcpy(temp, m_msg_err.c_str(), m_msg_err_len);
        temp += m_msg_err_len;

        int32_t m_msg_method_name_len = htonl(m_msg_method_name.size());
        memcpy(temp, &m_msg_method_name_len, sizeof(m_msg_method_name_len));
        temp += sizeof(m_msg_method_name_len);
        memcpy(temp, m_msg_method_name.c_str(), m_msg_method_name.size());
        temp += m_msg_method_name.size();

        int m_pb_data_len = m_pb_data.size();
        int32_t m_pb_data_len_len = htonl(m_pb_data_len);
        memcpy(temp, &m_pb_data_len_len, sizeof(m_pb_data_len_len));
        temp += sizeof(m_msg_id_len_len);
        memcpy(temp, m_pb_data.c_str(), m_pb_data_len);
        temp += m_pb_data_len;

        if (*(char *)temp == FLAGEND)
        {
            LOG_INFO("FLAGEND check success");
        }
        else
        {
            LOG_ERROR("FLAGEND check failed");
            return nullptr;
        }
        temp += 1;

        if (temp != data.get() + protocol_len)
        {
            LOG_ERROR("encode failed");
            return nullptr;
        }
        else
        {
            LOG_INFO("encode success");
            TcpBuffer::s_ptr buffer = std::make_shared<TcpBuffer>();
            buffer->writeFromChar(data, protocol_len);
            return buffer;
        }
    }

    Status PBProtocol::decode(TcpBuffer::s_ptr &buffer)
    {
        if (buffer->readAbleSize() == 0 || buffer->readAbleSize() == -1)
        {
            return Status(MyTinyRPC::Status::Code::CANCELLED, "buffer is empty, when start.");
        }
        int index = buffer->getReadIndex();
        int protocol_len = 0;
        while (true)
        {
            if (buffer->at(index++) == FLAGSTART)
            {
                if (buffer->readAbleSize(index - buffer->getReadIndex() + sizeof(int32_t)) == -1)
                {
                    return Status(MyTinyRPC::Status::Code::CANCELLED, "buffer not enough, when get protocol_len");
                }
                protocol_len = buffer->getInt32(index);
                if (buffer->readAbleSize(protocol_len) == -1)
                {
                    return Status(MyTinyRPC::Status::Code::CANCELLED, "buffer not enough, when get data");
                }
                if (buffer->at(index - 1 + protocol_len - 1) != FLAGEND)
                {
                    LOG_ERROR("decode failed");
                    continue;
                }
                index += sizeof(int32_t);
                break;
            }
            if (buffer->readAbleSize(index - buffer->getReadIndex()) == -1)
            {
                return Status(MyTinyRPC::Status::Code::CANCELLED, "buffer not enough, when out of start");
            }
        }
        // 读取总长度
        int m_msg_id_len = buffer->getInt32(index);
        index += sizeof(int32_t);
        std::string m_msg_id = buffer->getString(index, m_msg_id_len);
        index += m_msg_id_len;
        this->m_msg_id = m_msg_id;

        int m_msg_err_len = buffer->getInt32(index);
        index += sizeof(int32_t);
        std::string m_msg_err = buffer->getString(index, m_msg_err_len);
        index += m_msg_err_len;
        this->m_msg_err = m_msg_err;

        int m_msg_method_name_len = buffer->getInt32(index);
        index += sizeof(int32_t);
        std::string m_msg_method_name = buffer->getString(index, m_msg_method_name_len);
        index += m_msg_method_name_len;
        this->m_msg_method_name = m_msg_method_name;

        int m_pb_data_len = buffer->getInt32(index);
        index += sizeof(int32_t);
        std::string m_pb_data = buffer->getString(index, m_pb_data_len);
        index += m_pb_data_len;
        this->m_pb_data = m_pb_data;

        // 后面的标志位
        index += 1;

        buffer->moveReadIndex(index - buffer->getReadIndex());
        // buffer->adjustBuffer();
        return Status(MyTinyRPC::Status::Code::OK, "decode success");
    }
} // namespace MyTinyRPC
