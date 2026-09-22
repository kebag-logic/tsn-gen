#include <cstdio>
#include <cstring>
#include <limits>
volatile int one = 1;
static int noReturnValue(int x) { if (x > 5) return x; }   // flows off the end for x <= 5
int main(int argc, char** argv)
{
    volatile int maxInt = std::numeric_limits<int>::max();
    std::printf("mode %s started\n", argv[1]); std::fflush(stdout);
    if (!std::strcmp(argv[1], "signed-overflow")) std::printf("%d\n", maxInt + one);
    else if (!std::strcmp(argv[1], "missing-return")) std::printf("%d\n", noReturnValue(one));
    else if (!std::strcmp(argv[1], "unreachable")) { if (one) __builtin_unreachable(); }
    std::printf("mode %s completed\n", argv[1]);
    return 0;
}
