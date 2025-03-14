#include "Logger.hpp"
#include <iostream>
using namespace MEngine;
using namespace std;
int main()
{
    LOG_TRACE("Hello, World!\n");
    LOG_DEBUG("Hello, World! {}", 2021);
    LOG_INFO("Hello, World! {}, {}", 2021, true);
    LOG_WARN("Hello, World! {}, {}, {}", 2021, true, 3.14);
    LOG_ERROR("Hello, World! {}, {}, {}", 2021, true, 3.14);
    LOG_CRITICAL("Hello, World! {}, {}, {}, {}", 2021, true, 3.14, "MEngine");
    spdlog::shutdown();
    return 0;
}