#include "UUID.hpp"

namespace MEngine
{
UUID UUIDGenerator::operator()()
{

    UUID uuid;
    uuid.high = dis(gen);
    uuid.low = dis(gen);
    // byte 6
    uuid.high = (uuid.high & 0xFFFFFFFFFFFF0FFF) | (0x0000000000004000);
    // byte 8
    uuid.low = (uuid.low & 0x3FFFFFFFFFFFFFFF) | (0x8000000000000000);
    return uuid;
}
} // namespace MEngine