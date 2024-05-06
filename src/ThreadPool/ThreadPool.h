# ifndef MYTINYRPC_THREADPOOL_H
# define MYTINYRPC_THREADPOOL_H

#include <condition_variable>
#include <functional>
#include <future>
#include <queue>
#include <thread>

namespace MyTinyRPC
{
    // bug: 关键线程结束时，应该全部退出
    class ThreadPool
    {
        private:
            std::vector<std::thread> m_threads;
            std::queue<std::function<void()>> m_tasks;
            std::mutex m_mutex;
            std::condition_variable m_condVar;
            bool stop = false;
            bool shutdownMode = false;
            // TcpClietn：recv一个
            // TcpServer: 客户端连接占一个，处理buffer占一个，发送占一个
            // 普通任务处理一个
            // hack: 客户端连接是动态增加的
            int baseNumThreads = 5;

            ThreadPool(int numThreads);
            ~ThreadPool();

        public:
            template <typename F, typename... Args>
            auto enqueue(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
            {
                if (stop)
                {
                    throw std::runtime_error("Fatal: enqueue on stopped ThreadPool");
                }
                using return_type = decltype(f(args...));
                auto task = std::make_shared<std::packaged_task<return_type()>>(
                    std::bind(std::forward<F>(f), std::forward<Args>(args)...));
                std::future<return_type> res = task->get_future();
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    if (stop)
                    {
                        throw std::runtime_error("Fatal: enqueue on stopped ThreadPool");
                    }
                    m_tasks.emplace(
                        [task]()
                        {
                            (*task)();
                        });
                }
                m_condVar.notify_one();
                return res;
            }

            static ThreadPool &getInstance(int numThreads = std::thread::hardware_concurrency());

            void shutdown();
    };

} // namespace MyTinyRPC

# endif // MYTINYRPC_THREADPOOL_H