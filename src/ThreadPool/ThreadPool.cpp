#include "ThreadPool.h"
#include "Log/Log.h"

namespace MyTinyRPC
{
    ThreadPool::ThreadPool(int numThreads)
    {
        if (numThreads < baseNumThreads)
        {
            // LOG_ERROR("numThreads must be greater than or equal to %d", baseNumThreads);
            // exit(1);
            throw std::runtime_error("Fatal: numThreads must be greater than or equal to 5");
        }
        LOG_DEBUG("numThreads = %d", numThreads);
        for (int i = 0; i < numThreads; i++)
        {
            m_threads.emplace_back(
                [this]()
                {
                    while (true)
                    {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->m_mutex);
                            this->m_condVar.wait(lock,
                                                 [this]()
                                                 {
                                                     return this->stop || !this->m_tasks.empty();
                                                 });
                            if (this->stop && this->m_tasks.empty())
                            {
                                return;
                            }
                            task = std::move(this->m_tasks.front());
                            this->m_tasks.pop();
                        }
                        task();
                    }
                });
        }
    }

    ThreadPool::~ThreadPool()
    {
        shutdown();
    }

    ThreadPool &ThreadPool::getInstance(int numThreads)
    {
        static ThreadPool instance(numThreads);
        return instance;
    }

    void ThreadPool::shutdown()
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (stop)
            {
                return;
            }
            stop = true;
        }
        m_condVar.notify_all();
        for (std::thread &thread : m_threads)
        {
            thread.join();
        }
    }

} // namespace MyTinyRPC