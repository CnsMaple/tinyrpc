#ifndef STATUS_H
#define STATUS_H

#include <string>

namespace MyTinyRPC
{
    class Status
    {
        public:
            enum Code
            {
                UNKNOWN = 0,
                OK = 1,
                CANCELLED = 2,
                ERROR = 3,
            };

            Status(Code code, const std::string &message);

            Code getCode() const;

            const std::string &getMessage() const;

            bool operator()() const
            {
                return isOk();
            }

            operator bool() const
            {
                return isOk();
            }

            bool isOk() const;

        private:
            Code m_code;
            std::string m_message;
    };
} // namespace MyTinyRPC

#endif // STATUS_H