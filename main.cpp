#include <iostream>
#include <chrono>
#include <atomic>
#include "ThreadPool.hpp"

void Increment(std::atomic<std::size_t>& num)
{
    ++num;
}

int main(int, char**)
{
    std::cout << "Hello, world!\n";

    std::atomic<std::size_t> num = 0;

    ThreadPool pool(std::jthread::hardware_concurrency());
    for (std::size_t i = 0; i < 1000000; ++i)
    {
        pool.Enqueue(Executable(&Increment, std::ref(num)));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::cout << num << std::endl;
}
