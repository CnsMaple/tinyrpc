#ifndef LOG_H
#define LOG_H

#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

namespace MyTinyRPC
{

#define LOG_DEBUG(msg, ...)                                                                        \
    MyTinyRPC::Log::getInstance()->log(__FILE__,                                                   \
                                       __LINE__,                                                   \
                                       MyTinyRPC::formatString(msg, ##__VA_ARGS__),                \
                                       MyTinyRPC::LogLevel::DEBUG)

#define LOG_INFO(msg, ...)                                                                         \
    MyTinyRPC::Log::getInstance()->log(__FILE__,                                                   \
                                       __LINE__,                                                   \
                                       MyTinyRPC::formatString(msg, ##__VA_ARGS__),                \
                                       MyTinyRPC::LogLevel::INFO)

#define LOG_WARNING(msg, ...)                                                                      \
    MyTinyRPC::Log::getInstance()->log(__FILE__,                                                   \
                                       __LINE__,                                                   \
                                       MyTinyRPC::formatString(msg, ##__VA_ARGS__),                \
                                       MyTinyRPC::LogLevel::WARNING)

#define LOG_ERROR(msg, ...)                                                                        \
    MyTinyRPC::Log::getInstance()->log(__FILE__,                                                   \
                                       __LINE__,                                                   \
                                       MyTinyRPC::formatString(msg, ##__VA_ARGS__),                \
                                       MyTinyRPC::LogLevel::ERROR)

    enum LogLevel
    {
        UNKNOWN = 0,
        INFO = 1,
        WARNING = 2,
        ERROR = 3,
        DEBUG = 4,
        SILENT = 5,
    };

    template <typename... Args>
    std::string formatString(const char *str, Args &&...args)
    {

        int size = snprintf(nullptr, 0, str, args...);

        std::string result;
        if (size > 0)
        {
            result.resize(size);
            snprintf(&result[0], size + 1, str, args...);
        }

        return result;
    }

    class Log
    {
        private:
            std::mutex m_mutex;
            LogLevel m_logLevel;

        public:
            Log(LogLevel level = INFO);
            ~Log();
            typedef std::shared_ptr<Log> s_ptr;

            std::string getTime();

            std::string getLevel(LogLevel level);
            LogLevel getLevel(const std::string &level);

            static Log::s_ptr getInstance(LogLevel level = INFO);
            void log(const char *file, int line, std::string msg, LogLevel level = INFO);
    };
} // namespace MyTinyRPC

#endif // LOG_H