#include <iostream>
#include "Executable.hpp"

void Increment(int* in, int* out)
{
    *out = *in + 1;
}

int main(int, char**)
{
    std::cout << "Hello, world!\n";

    int in = 5, out;
    Executable task(&Increment, &in, &out);
    task();
    std::cout << out << std::endl;
}
