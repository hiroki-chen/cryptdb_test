#include <memory>
#include <iostream>
#include <vector>
#include <map>
#include <utility>

#include "monitor.hh"
#include "cryptohandlers.hh"
#include "helper.hh"
#include "tester.hh"

#ifdef DO_OPTIMIZATION

static const auto __ = []() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    return nullptr;
}();

#endif

int main(int argc, const char **argv)
{
// You can change it to 1 to complete the homework.
#if 0
    {
        const Monitor *const monitor = new Monitor("127.0.0.1", "root", "Chenhaobin123***", "3306");
        monitor->start();
    }
#endif

    const Tester *const tester =
        new Tester("127.0.0.1", "root", "Chenhaobin123***", "3306",
                   "input/test_case.txt", "input/china_mobile.csv", "log/log.txt");

    tester->start();
    // TODO: perform frequency attack on the encrypted columns.
    return 0;
}