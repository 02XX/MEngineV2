#pragma once
#include "UUID.hpp"
namespace MEngine
{
using UUID = UUID;
using UUIDGenerator = UUIDGenerator;
template <typename TKey = UUID> struct IComponent
{
    TKey id = UUIDGenerator()();
    bool isActive = true;
};
} // namespace MEngine