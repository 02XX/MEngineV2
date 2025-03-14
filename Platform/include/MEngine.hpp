#pragma once

#ifdef PLATFORM_WINDOWS
#ifdef MENGINE_EXPORT
#define MENGINE_API __declspec(dllexport)
#else
#define MENGINE_API __declspec(dllimport)
#endif
#elif PLATFORM_MACOS
#define MENGINE_API __attribute__((visibility("default")))
#elif PLATFORM_LINUX
#define MENGINE_API
#else
#error "Unknown platform!"
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <ranges>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
