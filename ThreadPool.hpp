#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <type_traits>
#include <functional>
#include <thread>
#include <mutex>
#include <future>
#include <condition_variable>

class ThreadPool
{
    template<typename Callable, typename... Args>
    using ReturnType = decltype(std::declval<Callable>()(std::declval<Args>()...));

    template<typename Callable, typename... Args>
    using CallableFuture = std::enable_if_t<std::is_invocable_v<Callable, Args...>, std::future<ReturnType<Callable, Args...>>>;

    using Task = std::function<void()>;

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
        std::call_once(m_creationFlag, [&] {
            const std::size_t arg { !numThreads ? std::jthread::hardware_concurrency() : numThreads };
            m_instance = new ThreadPool { arg };
        });
        return m_instance;
    }

    template<typename Callable, typename... Args>
    auto enqueue(Callable &&func, Args &&...args) -> CallableFuture<Callable, Args...>
    {
        auto wrapper { std::make_shared<std::packaged_task<ReturnType<Callable, Args...> ()>>(
            std::bind(std::forward<Callable>(func), std::forward<Args>(args)...)) };

        {
            std::lock_guard lock { m_taskMutex };
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
    std::vector<std::jthread> m_threads;

    std::condition_variable m_taskVar;
    bool m_stopping;
    std::mutex m_taskMutex;

    std::queue<Task> m_tasks;

private:
    static ThreadPool *m_instance;
    static std::once_flag m_creationFlag;

};

ThreadPool *ThreadPool::m_instance { nullptr };
std::once_flag ThreadPool::m_creationFlag {};