#include "ThreadPool.hpp"
#include <ranges>

ThreadPool::ThreadPool(std::size_t numThreads)
{
    m_threads.resize(numThreads);
    Start();
}

ThreadPool::~ThreadPool()
{
    Stop();
}

void ThreadPool::Enqueue(Executable&& task)
{
    {
        std::lock_guard lock(m_taskMutex);
        m_tasks.push(std::move(task));
    }
    m_taskVar.notify_one();
}

void ThreadPool::Start()
{
    std::ranges::for_each(std::ranges::begin(m_threads), std::ranges::end(m_threads), [&](auto& el) {
        el = std::jthread([&] {
            while (true)
            {
                Executable exec;

                {
                    std::unique_lock lock(m_taskMutex);
                    m_taskVar.wait(lock, [&] { return m_stopping || !m_tasks.empty(); } );
                    if (m_stopping) break;

                    exec = std::move(m_tasks.front());
                    m_tasks.pop();
                }

                exec();
            }
        }); 
    });
}

void ThreadPool::Stop() noexcept
{
    {
        std::lock_guard lock(m_taskMutex);
        m_stopping = true;
    }

    m_taskVar.notify_all();
}