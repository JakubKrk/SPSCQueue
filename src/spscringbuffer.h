#pragma once
#include <cstddef>
#include <optional>

template <typename T, typename Allocator = std::allocator<T>> class SpscRingBuffer
{
  public:
    using alloc_trait = std::allocator_traits<Allocator>;
    SpscRingBuffer() = delete;
    explicit SpscRingBuffer(std::size_t capacity, const Allocator &allocator = Allocator{})
        : _capacity(capacity), _allocator(allocator), _data(alloc_trait::allocate(_allocator, _capacity))
    {
    }
    ~SpscRingBuffer()
    {
        for (size_t i = 0; i < _size; ++i)
        {
            size_t idx = (_head + i) % _capacity;
            alloc_trait::destroy(_allocator, _data + idx);
        }
        alloc_trait::deallocate(_allocator, _data, _capacity);
    };
    template <typename... Args> [[nodiscard]] bool try_push(Args &&...args)
    {
        if (isFull())
            return false;
        alloc_trait::construct(_allocator, _data + (_head + _size) % _capacity, std::forward<Args>(args)...);
        ++_size;
        return true;
    }
    std::optional<T> pop()
    {
        if (isEmpty())
            return std::nullopt;

        T element = std::move(_data[_head]);
        alloc_trait::destroy(_allocator, _data + _head);
        _head = (_head + 1) % _capacity;
        --_size;
        return std::move(element);
    }

    bool isEmpty() const
    {
        return _size == 0;
    }
    bool isFull() const
    {
        return _size == _capacity;
    }
    std::size_t size() const
    {
        return _size;
    }

    void reset()
    {
        for (std::size_t i = 0; i < _size; ++i)
            alloc_trait::destroy(_allocator, _data + (_head + i) % _capacity);
        _head = 0;
        _size = 0;
    }

  private:
    Allocator _allocator;
    std::size_t _capacity{0U};
    std::size_t _head{0U};
    std::size_t _size{0U};
    T *_data{nullptr};
};
