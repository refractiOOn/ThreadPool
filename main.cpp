#include <iostream>
#include <chrono>
#include "ThreadPool.hpp"

void Increment(int* in, int* out)
{
    *out = *in + 1;
}

int main(int, char**)
{
    std::cout << "Hello, world!\n";

    int in = 5, out;
    Executable task(&Increment, &in, &out);

    ThreadPool pool(std::jthread::hardware_concurrency());
    pool.Enqueue(std::move(task));

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << out << std::endl;
}
