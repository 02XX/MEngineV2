#include "UUID.hpp"

namespace MEngine
{
UUID UUIDGenerator::operator()()
{

    UUID uuid;
    uuid.high = dis(gen);
    uuid.low = dis(gen);
    return uuid;
}
} // namespace MEngine