#include <iostream>
#include <atomic>
#include <vector>
#include "ThreadPool.hpp"

int Plus(int a, int b, std::atomic<std::size_t>& totalSum)
{
    int localSum = a + b;
    totalSum += localSum;
    return localSum;
}

int main(int, char**)
{
    std::cout << "Hello, world!\n";

    std::atomic<std::size_t> totalSum = 0;
    std::vector<std::future<int>> futures;

    ThreadPool pool(std::jthread::hardware_concurrency());
    for (int i = 0; i < 100000; ++i)
    {
        auto res = pool.Enqueue(Plus, 10, 20, std::ref(totalSum));
        futures.push_back(std::move(res));
    }

    for (auto& el : futures) el.get(); // waiting for finish
    
    std::cout << "Total sum: " << totalSum << std::endl;
}
