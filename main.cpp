#include <iostream>
#include <atomic>
#include <vector>
#include "ThreadPool.hpp"

int plus(int a, int b, std::atomic<std::size_t> &totalSum)
{
    int localSum { a + b };
    totalSum += localSum;
    return localSum;
}

int main(int, char**)
{
    std::atomic<std::size_t> totalSum { 0 };
    std::vector<std::future<int>> futures {};

    ThreadPool *pool { ThreadPool::getInstance() };
    for (int i { 0 }; i < 100; ++i)
    {
        auto res { pool->enqueue(plus, 10, 20, std::ref(totalSum)) };
        futures.push_back(std::move(res));
    }

    for (auto &el : futures) std::cout << "Local sum: " << el.get() << std::endl;
    
    std::cout << "Total sum: " << totalSum << std::endl;
}
