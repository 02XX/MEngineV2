#pragma once
#include <atomic>
#include <cstdint>
#include <vulkan/vulkan.hpp>
namespace MEngine
{

template <typename T, typename Dispatch> class SharedHandleTraits;

template <typename Dispatch> class SharedHandleTraits<vk::PipelineLayout, Dispatch>
{
  public:
    using deleter = vk::detail::ObjectDestroy<vk::Device, Dispatch>;
};

template <typename Type, typename Dispatch> class SharedHandle : public SharedHandleTraits<Type, Dispatch>::deleter
{
  private:
    using Deleter = typename SharedHandleTraits<Type, Dispatch>::deleter;

  public:
    using element_type = Type;

    SharedHandle() : Deleter(), m_value(), mCounter(nullptr)
    {
    }

    explicit SharedHandle(Type const &value, Deleter const &deleter = Deleter()) VULKAN_HPP_NOEXCEPT
        : Deleter(deleter),
          m_value(value),
          mCounter(value ? new std::atomic<int64_t>(1) : nullptr)
    {
    }

    SharedHandle(SharedHandle const &other) : Deleter(other), m_value(other.m_value), mCounter(other.mCounter)
    {
        if (mCounter)
        {
            mCounter->fetch_add(1, std::memory_order_relaxed);
        }
    }

    SharedHandle(SharedHandle &&other) VULKAN_HPP_NOEXCEPT : Deleter(std::move(static_cast<Deleter &>(other))),
                                                             m_value(other.m_value),
                                                             mCounter(other.mCounter)
    {
        other.m_value = nullptr;
        other.mCounter = nullptr;
    }

    ~SharedHandle() VULKAN_HPP_NOEXCEPT
    {
        if (mCounter->fetch_sub(1, std::memory_order_acq_rel) == 1 && m_value)
        {
            this->destroy(m_value);
            delete mCounter;
        }
    }

    SharedHandle &operator=(SharedHandle const &other)
    {

        if (mCounter && mCounter->fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            this->destroy(m_value);
            delete mCounter;
        }
        m_value = other.m_value;
        mCounter = other.mCounter;
        *static_cast<Deleter &>(this) = other;
        if (mCounter)
        {
            mCounter->fetch_add(1, std::memory_order_relaxed);
        }
        return *this;
    }

    SharedHandle &operator=(SharedHandle &&other) VULKAN_HPP_NOEXCEPT
    {
        if (mCounter && mCounter->fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            this->destroy(m_value);
            delete mCounter;
        }
        m_value = other.m_value;
        mCounter = other.mCounter;
        *static_cast<Deleter *>(this) = std::move(static_cast<Deleter &>(other));

        other.m_value = nullptr;
        other.mCounter = nullptr;
        return *this;
    }

    explicit operator bool() const VULKAN_HPP_NOEXCEPT
    {
        return m_value.operator bool();
    }

#if defined(VULKAN_HPP_SMART_HANDLE_IMPLICIT_CAST)
    operator Type() const VULKAN_HPP_NOEXCEPT
    {
        return m_value;
    }
#endif

    Type const *operator->() const VULKAN_HPP_NOEXCEPT
    {
        return &m_value;
    }

    Type *operator->() VULKAN_HPP_NOEXCEPT
    {
        return &m_value;
    }

    Type const &operator*() const VULKAN_HPP_NOEXCEPT
    {
        return m_value;
    }

    Type &operator*() VULKAN_HPP_NOEXCEPT
    {
        return m_value;
    }

    const Type &get() const VULKAN_HPP_NOEXCEPT
    {
        return m_value;
    }

    Type &get() VULKAN_HPP_NOEXCEPT
    {
        return m_value;
    }

    void reset(Type const &value = Type()) VULKAN_HPP_NOEXCEPT
    {
        if (m_value != value)
        {
            if (mCounter && mCounter->fetch_sub(1, std::memory_order_acq_rel) == 1)
            {
                this->destroy(m_value);
                delete mCounter;
            }
            if (m_value)
            {
                this->destroy(m_value);
            }
            m_value = value;
            mCounter = value ? new std::atomic<int64_t>(1) : nullptr;
        }
    }

    Type release() VULKAN_HPP_NOEXCEPT
    {
        Type value = m_value;
        m_value = nullptr;
        return value;
    }
    int useCount() const VULKAN_HPP_NOEXCEPT
    {
        return mCounter ? mCounter->load(std::memory_order_relaxed) : 0;
    }
    void swap(SharedHandle<Type, Dispatch> &rhs) VULKAN_HPP_NOEXCEPT
    {
        std::swap(m_value, rhs.m_value);
        std::swap(mCounter, rhs.mCounter);
        std::swap(static_cast<Deleter &>(*this), static_cast<Deleter &>(rhs));
    }

  private:
    Type m_value;
    std::atomic<int64_t> *mCounter;
};

using SharedPipelineLayout = SharedHandle<vk::PipelineLayout, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE>;

} // namespace MEngine
