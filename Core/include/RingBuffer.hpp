#pragma once
#include <iterator>
#include <stdexcept>

namespace MEngine
{

template <typename T> class RingBuffer
{
  private:
    T *buffer_;
    size_t capacity_;
    size_t size_;
    size_t head_;
    size_t tail_;

  public:
    explicit RingBuffer(size_t capacity) : capacity_(capacity), size_(0), head_(0), tail_(0)
    {
        if (capacity == 0)
        {
            throw std::invalid_argument("Capacity must be greater than 0");
        }
        buffer_ = new T[capacity];
    }

    ~RingBuffer()
    {
        delete[] buffer_;
    }

    // Push with overwrite
    void PushOverwrite(const T &item)
    {
        if (size_ == capacity_)
        {
            buffer_[tail_] = item;
            tail_ = (tail_ + 1) % capacity_;
            head_ = (head_ + 1) % capacity_;
        }
        else
        {
            buffer_[tail_] = item;
            tail_ = (tail_ + 1) % capacity_;
            size_++;
        }
    }

    // Push without overwrite
    void Push(const T &item)
    {
        if (size_ == capacity_)
        {
            throw std::runtime_error("RingBuffer is full");
        }
        buffer_[tail_] = item;
        tail_ = (tail_ + 1) % capacity_;
        size_++;
    }

    T Pop()
    {
        if (size_ == 0)
        {
            throw std::runtime_error("RingBuffer is empty");
        }
        T item = buffer_[head_];
        head_ = (head_ + 1) % capacity_;
        size_--;
        return item;
    }
    T Front() const
    {
        if (size_ == 0)
        {
            throw std::runtime_error("RingBuffer is empty");
        }
        return buffer_[head_];
    }
    size_t Size() const
    {
        return size_;
    }
    size_t Capacity() const
    {
        return capacity_;
    }
    bool IsEmpty() const
    {
        return size_ == 0;
    }
    bool IsFull() const
    {
        return size_ == capacity_;
    }

    // Iterator class
    class Iterator
    {
      private:
        const RingBuffer *buffer_;
        size_t index_;

      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T *;
        using reference = T &;

        Iterator(const RingBuffer *buffer, size_t index) : buffer_(buffer), index_(index)
        {
        }

        T &operator*() const
        {
            if (index_ >= buffer_->size_)
            {
                throw std::out_of_range("Iterator out of range");
            }
            return buffer_->buffer_[(buffer_->head_ + index_) % buffer_->capacity_];
        }

        Iterator &operator++()
        {
            ++index_;
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const Iterator &other) const
        {
            return buffer_ == other.buffer_ && index_ == other.index_;
        }

        bool operator!=(const Iterator &other) const
        {
            return !(*this == other);
        }
    };

    Iterator begin() const
    {
        return Iterator(this, 0);
    }

    Iterator end() const
    {
        return Iterator(this, size_);
    }
};
} // namespace MEngine