#pragma once

#include "fmt/format.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>

namespace MEngine
{
class UUID final
{
    friend class std::hash<MEngine::UUID>;
    friend class fmt::formatter<MEngine::UUID>;
    friend class UUIDGenerator;

  private:
    uint64_t high; // 64 bits
    uint64_t low;  // 64 bits
  public:
    // Construct from string in format "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
    UUID(const std::string &uuid)
    {
        if (uuid.empty())
        {
            high = 0;
            low = 0;
            return;
        }

        // Remove hyphens and convert to 128-bit value
        std::string hexStr;
        for (char c : uuid)
        {
            if (c != '-')
            {
                hexStr += c;
            }
        }

        if (hexStr.length() != 32)
        {
            high = 0;
            low = 0;
            return;
        }

        high = std::stoull(hexStr.substr(0, 16), nullptr, 16);
        low = std::stoull(hexStr.substr(16, 16), nullptr, 16);
    }
    UUID() : high(0), low(0)
    {
    }
    // Convert to string in format "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
    std::string ToString() const
    {
        if (IsEmpty())
        {
            return "";
        }

        std::ostringstream oss;
        oss << std::hex << std::setfill('0');

        // Format high bits (64 bits)
        oss << std::setw(16) << high;
        std::string highStr = oss.str();
        oss.str("");

        // Format low bits (64 bits)
        oss << std::setw(16) << low;
        std::string lowStr = oss.str();

        // Combine and insert hyphens
        std::string result = highStr + lowStr;
        return result.substr(0, 8) + "-" + result.substr(8, 4) + "-" + result.substr(12, 4) + "-" +
               result.substr(16, 4) + "-" + result.substr(20, 12);
    }

    bool IsEmpty() const
    {
        return high == 0 && low == 0;
    }

    bool operator==(const UUID &other) const
    {
        return high == other.high && low == other.low;
    }

    bool operator!=(const UUID &other) const
    {
        return !(*this == other);
    }

    // For use in ordered containers
    bool operator<(const UUID &other) const
    {
        if (high != other.high)
        {
            return high < other.high;
        }
        return low < other.low;
    }
};

class UUIDGenerator final
{
  private:
    std::random_device rd;
    std::mt19937_64 gen;
    std::uniform_int_distribution<uint64_t> dis;

  public:
    UUIDGenerator() : gen(rd()), dis(0, UINT64_MAX)
    {
    }

    UUID operator()();
};
} // namespace MEngine
namespace std
{
template <> struct hash<MEngine::UUID>
{
    size_t operator()(const MEngine::UUID &id) const
    {
        // FNV-1a hash function
        size_t hash = 2166136261u;
        size_t prime = 16777619;
        hash ^= (id.high >> 32) & 0xFFFFFFFF;
        hash *= prime;
        hash ^= (id.high & 0xFFFFFFFF);
        hash *= prime;
        hash ^= (id.low >> 32) & 0xFFFFFFFF;
        hash *= prime;
        hash ^= (id.low & 0xFFFFFFFF);
        hash *= prime;
        return hash;
    }
};
} // namespace std
template <> struct fmt::formatter<MEngine::UUID>
{
    constexpr auto parse(fmt::format_parse_context &ctx) -> decltype(ctx.begin())
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(const MEngine::UUID &id, FormatContext &ctx) const -> decltype(ctx.out())
    {
        // Convert to string in format "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
        std::string uuidStr = id.ToString();
        // Format the UUID string
        return fmt::format_to(ctx.out(), "{}", uuidStr);
    }
};