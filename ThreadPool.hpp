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
public:
    typedef std::function<void()> Task;

    ThreadPool(std::size_t numThreads)
    {
        m_threads.resize(numThreads);
        Start();
    }
    ~ThreadPool()
    {
        Stop();
    }

    template<typename Func, typename... Args>
    auto Enqueue(Func&& func, Args&&... args) -> std::future<decltype(func(std::declval<Args>()...))>
    {
        auto wrapper = std::make_shared<std::packaged_task<decltype(func(std::declval<Args>()...)) ()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

        {
            std::unique_lock lock(m_taskMutex);
            m_tasks.emplace([=] {
                (*wrapper)();
            });
        }
        m_taskVar.notify_one();

        return wrapper->get_future();
    }

private:
    void Start()
    {
        std::ranges::for_each(std::ranges::begin(m_threads), std::ranges::end(m_threads), [&](auto& el) {
            el = std::jthread([&] {
                while (true)
                {
                    Task task;

                    {
                        std::unique_lock lock(m_taskMutex);
                        m_taskVar.wait(lock, [&] { return m_stopping || !m_tasks.empty(); } );
                        if (m_stopping && m_tasks.empty()) break;

                        task = std::move(m_tasks.front());
                        m_tasks.pop();
                    }

                    task();
                }
            }); 
        });
    }
    void Stop() noexcept
    {
        {
            std::lock_guard lock(m_taskMutex);
            m_stopping = true;
        }

        m_taskVar.notify_all();
    }

private:
    std::vector<std::jthread> m_threads;

    std::condition_variable m_taskVar;
    bool m_stopping = false;
    std::mutex m_taskMutex;

    std::queue<Task> m_tasks;

};