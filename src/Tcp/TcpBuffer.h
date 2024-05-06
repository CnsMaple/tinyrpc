#ifndef TCPBUFFER_H
#define TCPBUFFER_H

#include <memory>
#include <string>
#include <vector>

namespace MyTinyRPC
{
    class TcpBuffer
    {
        public:
            typedef std::shared_ptr<TcpBuffer> s_ptr;

            TcpBuffer(int size = 4096);

            void writeFromChar(const char *data, int len);

            void writeFromChar(std::shared_ptr<char> data, int len);

            int getInt32(int loc);

            std::string getString(int loc, int len);

            void moveWriteIndex(int len);

            int writeAbleSize(int rel_loc = 0);

            int readAbleSize(int rel_loc = 0);

            void moveReadIndex(int len);

            void resize(int size);

            char *getBuffer();

            int getReadIndex();

            int getWriteIndex();

            char at(int loc);

            // std::shared_ptr<char> getFromBuffer(int loc, int len);

        private:
            // std::vector<char> m_buffer;
            std::shared_ptr<char> m_buffer;
            int m_buffer_size;

            int m_write_index;
            int m_read_index;
            // 如果这个是true，那么两个index相等的时候，说明buffer已经写满了
            bool m_flag_write_before_read;
    };
} // namespace MyTinyRPC

#endif // TCPBUFFER_H