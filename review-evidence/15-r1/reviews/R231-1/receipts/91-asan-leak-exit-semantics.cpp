#include <cstdio>
#include <cstdlib>
void* volatile sink;
int main()
{
    sink = std::malloc(64);
    sink = nullptr;   // the only pointer is dropped: a leak at exit
    std::printf("leak mode completed\n");
    return 0;
}
