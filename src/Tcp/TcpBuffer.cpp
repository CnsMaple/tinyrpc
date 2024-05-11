#include "TcpBuffer.h"
#include <arpa/inet.h>
#include <string.h>
#include <stdexcept>

namespace MyTinyRPC
{

    TcpBuffer::TcpBuffer(int size)
    {
        m_buffer_size = size;
        m_buffer = std::shared_ptr<char>(new char[m_buffer_size], std::default_delete<char[]>());
        m_write_index = 0;
        m_read_index = 0;
        m_flag_write_before_read = false;
    }

    void TcpBuffer::writeFromChar(const char *data, int len)
    {
        if (writeAbleSize(len) == -1)
        {
            resize((m_buffer_size + len) * 10);
        }

        if (m_write_index >= m_read_index)
        {
            if (m_flag_write_before_read && m_read_index == m_write_index)
            {
                throw std::runtime_error(
                    "Fatal: Writing before reading is not allowed, not enough space.");
            }

            if (m_write_index + len > m_buffer_size)
            {
                memcpy(m_buffer.get() + m_write_index, data, m_buffer_size - m_write_index);
                memcpy(m_buffer.get(),
                       data + m_buffer_size - m_write_index,
                       len - (m_buffer_size - m_write_index));
            }
            else
            {
                memcpy(m_buffer.get() + m_write_index, data, len);
            }
        }
        else if (m_write_index < m_read_index)
        {
            memcpy(m_buffer.get() + m_write_index, data, len);
        }
        moveWriteIndex(len);
    }

    void TcpBuffer::writeFromChar(std::shared_ptr<char> data, int len)
    {
        writeFromChar(data.get(), len);
    }

    int TcpBuffer::getInt32(int loc)
    {
        int len = sizeof(int32_t);
        char num[len];
        int temp = loc % m_buffer_size;
        int temp2 = (loc + len) % m_buffer_size;
        if (m_write_index > m_read_index)
        {
            if (loc > m_write_index || loc + len > m_write_index || loc < m_read_index)
            {
                throw std::runtime_error("Fatal: getInt32() loc is out of range.");
            }
            else
            {
                memcpy(num, m_buffer.get() + loc, len);
            }
        }
        else if (m_write_index <= m_read_index)
        {
            if ((loc > m_write_index && loc < m_read_index)
                || (temp > m_write_index && temp < m_read_index)
                || (temp2 > m_write_index && temp2 < m_read_index)
                || (!m_flag_write_before_read && m_write_index == m_read_index))
            {
                throw std::runtime_error("Fatal: getInt32() loc is out of range.");
            }
            else
            {
                if (loc >= m_buffer_size)
                {
                    memcpy(num, m_buffer.get() + temp, len);
                }
                else
                {
                    if (loc + len >= m_buffer_size)
                    {
                        memcpy(num, m_buffer.get() + loc, m_buffer_size - loc);
                        memcpy(num + m_buffer_size - loc,
                               m_buffer.get(),
                               len - (m_buffer_size - loc));
                    }
                    else
                    {
                        memcpy(num, m_buffer.get() + loc, len);
                    }
                }
            }
        }

        return ntohl(*(int32_t *)(num));
    }

    std::string TcpBuffer::getString(int loc, int len)
    {
        char str[len];
        int temp = loc % m_buffer_size;
        int temp2 = (loc + len) % m_buffer_size;
        if (m_write_index > m_read_index)
        {
            if (loc > m_write_index || loc + len > m_write_index || loc < m_read_index)
            {
                throw std::runtime_error("Fatal: getString() loc is out of range.");
            }
            else
            {
                memcpy(str, m_buffer.get() + loc, len);
            }
        }
        else if (m_write_index <= m_read_index)
        {
            if ((loc > m_write_index && loc < m_read_index)
                || (temp > m_write_index && temp < m_read_index)
                || (temp2 > m_write_index && temp2 < m_read_index)
                || (!m_flag_write_before_read && m_write_index == m_read_index))
            {
                throw std::runtime_error("Fatal: getString() loc is out of range.");
            }
            else
            {
                if (loc >= m_buffer_size)
                {
                    memcpy(str, m_buffer.get() + temp, len);
                }
                else
                {
                    if (loc + len >= m_buffer_size)
                    {
                        memcpy(str, m_buffer.get() + loc, m_buffer_size - loc);
                        memcpy(str + m_buffer_size - loc,
                               m_buffer.get(),
                               len - (m_buffer_size - loc));
                    }
                    else
                    {
                        memcpy(str, m_buffer.get() + loc, len);
                    }
                }
            }
        }
        return std::string(str, len);
    }

    // hack:
    // 不要拿这个函数去比较，1和-1是完全不一样的情况，而结果可能一样，比较失败的使用用-1，否则均为true
    int TcpBuffer::writeAbleSize(int rel_loc)
    {
        if (rel_loc > m_buffer_size)
        {
            return -1;
        }
        if (m_write_index > m_read_index)
        {
            if (rel_loc > m_buffer_size - m_write_index + m_read_index)
            {
                return -1;
            }
            else
            {
                return m_buffer_size - m_write_index + m_read_index - rel_loc;
            }
        }
        else if (m_write_index < m_read_index)
        {
            if (rel_loc > m_read_index - m_write_index)
            {
                return -1;
            }
            else
            {
                return m_read_index - m_write_index - rel_loc;
            }
        }
        else
        {
            // 这个是相等的情况
            if (m_flag_write_before_read)
            {
                return -1;
            }
            else
            {
                return m_buffer_size - rel_loc;
            }
        }
    }

    // hack:
    // 不要拿这个函数去比较，1和-1是完全不一样的情况，而结果可能一样，比较失败的使用用-1，否则均为true
    // loc 是一个相对的位置，不包含m_read_index
    int TcpBuffer::readAbleSize(int rel_loc)
    {
        if (rel_loc > m_buffer_size)
        {
            return -1;
        }
        if (m_write_index > m_read_index)
        {
            if (rel_loc > m_write_index - m_read_index)
            {
                return -1;
            }
            else
            {
                return m_write_index - m_read_index - rel_loc;
            }
        }
        else if (m_write_index < m_read_index)
        {
            if (rel_loc > m_buffer_size - m_read_index + m_write_index)
            {
                return -1;
            }
            else
            {
                return m_buffer_size - m_read_index + m_write_index - rel_loc;
            }
        }
        else
        {
            if (m_flag_write_before_read)
            {
                return m_buffer_size - rel_loc;
            }
            else
            {
                return -1;
            }
        }
    }

    void TcpBuffer::moveWriteIndex(int len)
    {
        if (writeAbleSize(len) != -1)
        {
            if (writeAbleSize(len) == 0)
            {
                m_flag_write_before_read = true;
            }
            m_write_index = (m_write_index + len) % m_buffer_size;
        }
        else
        {
            throw std::runtime_error("Fatal: moveWriteIndex() is out of range");
        }
    }

    void TcpBuffer::moveReadIndex(int len)
    {
        if (readAbleSize(len) != -1)
        {
            if (readAbleSize(len) == 0)
            {
                m_flag_write_before_read = false;
            }
            m_read_index = (m_read_index + len) % m_buffer_size;
        }
        else
        {
            throw std::runtime_error("Fatal: moveReadIndex() is out of range");
        }
    }

    void TcpBuffer::resize(int size)
    {
        std::shared_ptr<char> m_buffer_new = std::shared_ptr<char>(new char[size]);
        // memcpy(m_buffer_new.get(), getBuffer() + m_read_index, m_write_index - m_read_index);
        // m_write_index = m_write_index - m_read_index;
        // m_read_index = 0;
        // m_buffer.swap(m_buffer_new);
        int old_read_size = readAbleSize();
        // 需要使用memcpy
        // for (int i = 0; i < old_read_size; i++)
        // {
        //     m_buffer_new.get()[i] = getBuffer()[(m_read_index + i) % m_buffer_size];
        //     // m_buffer_new.get()[i] = at((m_read_index + i) % m_buffer_size);
        // }
        if (m_write_index > m_read_index)
        {
            memcpy(m_buffer_new.get(), getBuffer() + m_read_index, readAbleSize());
        }
        else if (m_write_index <= m_read_index)
        {
            if (!m_flag_write_before_read && m_write_index == m_read_index)
            {
                throw std::runtime_error("Fatal: the buffer is empty");
            }
            memcpy(m_buffer_new.get(), getBuffer() + m_read_index, m_buffer_size - m_read_index);
            memcpy(m_buffer_new.get() + m_buffer_size - m_read_index, getBuffer(), m_write_index);
        }

        if (old_read_size > size)
        {
            throw std::runtime_error("Fatal: the size is too small");
        }
        m_write_index = old_read_size;
        m_read_index = 0;
        if (m_write_index == m_read_index)
        {
            m_flag_write_before_read = false;
        }
        m_buffer.swap(m_buffer_new);
        m_buffer_size = size;
    }

    char *TcpBuffer::getBuffer()
    {
        return m_buffer.get();
    }

    int TcpBuffer::getReadIndex()
    {
        return m_read_index;
    }

    int TcpBuffer::getWriteIndex()
    {
        return m_write_index;
    }

    char TcpBuffer::at(int loc)
    {
        // if (readAbleSize(loc - m_read_index) == -1)
        // {
        //     throw std::runtime_error("Fatal: the at() is out of range");
        // }
        // if (m_write_index > m_read_index)
        // {
        //     if (!(loc >= m_read_index && loc < m_write_index))
        //     {
        //         throw std::runtime_error("Fatal: the at() is out of range");
        //     }
        // }
        // else if (m_write_index <= m_read_index)
        // {
        //     if (!m_flag_write_before_read && m_write_index == m_read_index)
        //     {
        //         throw std::runtime_error("Fatal: the buffer is empty");
        //     }
        //     if (loc >= m_buffer_size)
        //     {
        //         int temp = loc % m_buffer_size;
        //         if (temp > m_write_index)
        //         {
        //             throw std::runtime_error("Fatal: the at() is out of range");
        //         }
        //     }
        //     else
        //     {
        //         if (loc < m_read_index)
        //         {
        //             throw std::runtime_error("Fatal: the at() is out of range");
        //         }
        //     }
        // }
        return getBuffer()[(loc) % m_buffer_size];
    }

} // namespace MyTinyRPC