#include "Log.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <time.h>

namespace MyTinyRPC
{
    Log::Log(LogLevel level) : m_logLevel(level)
    {
    }

    Log::~Log()
    {
    }

    void Log::log(const char *file, int line, std::string msg, LogLevel level)
    {
        if (level >= m_logLevel)
        {
            std::stringstream logSs;
            logSs << "[" << MyTinyRPC::Log::getInstance()->getTime() << "]"
                  << " "
                  << "[" << MyTinyRPC::Log::getInstance()->getLevel(level) << "]"
                  << " "
                  << "[" << std::this_thread::get_id() << ": " << file << ":" << line << "]"
                  << " : " << msg;

            {
                std::unique_lock<std::mutex> lock(m_mutex);
                std::cout << logSs.str() << std::endl;
            }
        }
    }

    std::string Log::getTime()
    {
        time_t now = time(0);
        struct tm tstruct;
        char buf[80];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
        return buf;
    }

    std::string Log::getLevel(LogLevel level)
    {
        if (level == INFO)
        {
            return "INFO";
        }
        else if (level == WARNING)
        {
            return "WARNING";
        }
        else if (level == ERROR)
        {
            return "ERROR";
        }
        else if (level == DEBUG)
        {
            return "DEBUG";
        }
        else
        {
            return "UNKNOWN";
        }
    }

    LogLevel Log::getLevel(const std::string &level)
    {
        if (level == "INFO")
        {
            return INFO;
        }
        else if (level == "WARNING")
        {
            return WARNING;
        }
        else if (level == "ERROR")
        {
            return ERROR;
        }
        else if (level == "DEBUG")
        {
            return DEBUG;
        }
        else
        {
            return UNKNOWN;
        }
    }

    static Log::s_ptr global_log = nullptr;

    Log::s_ptr Log::getInstance(MyTinyRPC::LogLevel level)
    {
        if (!global_log)
        {
            global_log = std::make_shared<Log>(level);
        }
        return global_log;
    }
} // namespace MyTinyRPC