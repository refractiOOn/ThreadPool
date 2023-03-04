#include "Executable.hpp"
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

class ThreadPool
{
public:
    ThreadPool(std::size_t numThreads);
    ~ThreadPool();

    void Enqueue(Executable&& task);

private:
    void Start();
    void Stop() noexcept;

private:
    std::vector<std::jthread> m_threads;

    std::condition_variable m_taskVar;
    bool m_stopping = false;
    std::mutex m_taskMutex;

    std::queue<Executable> m_tasks;

};