#pragma once
#include "MEngine.hpp"

namespace MEngine
{
class NoCopyable
{
  protected:
    NoCopyable() = default;
    virtual ~NoCopyable() = default;
    NoCopyable(const NoCopyable &) = delete;
    NoCopyable &operator=(const NoCopyable &) = delete;
    NoCopyable(NoCopyable &&) = delete;
    NoCopyable &operator=(NoCopyable &&) = delete;
};
} // namespace MEngine