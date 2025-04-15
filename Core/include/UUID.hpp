#pragma once
#include "fmt/format.h"
#include <cstdint>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>

namespace MEngine
{
class UUID final
{
  private:
    static constexpr int HEX_DIGITS_PER_BYTE = 2;
    static constexpr int UUID_STRING_LENGTH = 36; // 32 hex digits + 4 hyphens

  public:
    uint64_t high;
    uint64_t low;

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

    // Default constructor creates empty UUID
    UUID() : high(0), low(0)
    {
    }

    // Copy/move constructors and assignment operators
    UUID(const UUID &other) = default;
    UUID &operator=(const UUID &other) = default;
    UUID(UUID &&other) = default;
    UUID &operator=(UUID &&other) = default;
    ~UUID() = default;

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
        return std::hash<uint64_t>{}(id.high) ^ (std::hash<uint64_t>{}(id.low) << 1);
    }
};
} // namespace std
template <> struct fmt::formatter<MEngine::UUID>
{
    // 解析格式说明符（此处无特殊处理）
    constexpr auto parse(fmt::format_parse_context &ctx) -> decltype(ctx.begin())
    {
        return ctx.begin();
    }

    // 定义UUID的格式化逻辑
    template <typename FormatContext>
    auto format(const MEngine::UUID &id, FormatContext &ctx) const -> decltype(ctx.out())
    {
        // 示例：将UUID格式化为两个16位十六进制数（根据实际需求调整）
        return fmt::format_to(ctx.out(), "{:016X}-{:016X}", id.high, id.low);
    }
};