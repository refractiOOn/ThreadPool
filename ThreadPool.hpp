#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <future>
#include <memory>
#include <functional>
#include <condition_variable>

class ThreadPool
{
    typedef std::function<void()> Task;

    ThreadPool(std::size_t numThreads) :
        m_stopping { false }
    {
        m_threads.resize(numThreads);
        start();
    }

public:
    ~ThreadPool()
    {
        stop();
    }

    static ThreadPool *getInstance(const std::size_t numThreads = 0)
    {
        std::lock_guard lock { m_creationMutex };
        if (!m_instance)
        {
            const std::size_t arg { !numThreads ? std::jthread::hardware_concurrency() : numThreads };
            m_instance = new ThreadPool(arg);
        }
        return m_instance;
    }

    template<typename Func, typename... Args>
    auto enqueue(Func &&func, Args &&...args) -> std::future<decltype(func(std::declval<Args>()...))>
    {
        auto wrapper { std::make_shared<std::packaged_task<decltype(func(std::declval<Args>()...)) ()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...)) };

        {
            std::unique_lock lock { m_taskMutex };
            m_tasks.emplace([=] {
                (*wrapper)();
            });
        }
        m_taskVar.notify_one();

        return wrapper->get_future();
    }

private:
    void start()
    {
        std::for_each(std::begin(m_threads), std::end(m_threads), [&](auto &el) {
            el = std::jthread {[&] {
                while (true)
                {
                    Task task {};

                    {
                        std::unique_lock lock { m_taskMutex };
                        m_taskVar.wait(lock, [&] { return m_stopping || !m_tasks.empty(); } );
                        if (m_stopping && m_tasks.empty()) break;

                        task = std::move(m_tasks.front());
                        m_tasks.pop();
                    }

                    task();
                }
            }};
        });
    }
    void stop() noexcept
    {
        {
            std::lock_guard lock { m_taskMutex };
            m_stopping = true;
        }

        m_taskVar.notify_all();
    }

private:
    std::vector<std::jthread> m_threads {};

    std::condition_variable m_taskVar {};
    bool m_stopping {};
    std::mutex m_taskMutex {};

    std::queue<Task> m_tasks {};

private:
    static ThreadPool *m_instance;
    static std::mutex m_creationMutex;

};

ThreadPool *ThreadPool::m_instance { nullptr };
std::mutex ThreadPool::m_creationMutex {};