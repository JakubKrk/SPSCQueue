#pragma once
#include <atomic>
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
        std::size_t pop = _popper.load(std::memory_order_relaxed);
        std::size_t push = _pusher.load(std::memory_order_relaxed);
        for (; pop != push; ++pop)
            alloc_trait::destroy(_allocator, get(pop));
        alloc_trait::deallocate(_allocator, _data, _capacity);
    }

    template <typename... Args> [[nodiscard]] bool try_push(Args &&...args)
    {
        std::size_t push = _pusher.load(std::memory_order_relaxed);
        std::size_t pop = _popper.load(std::memory_order_acquire);
        if (push - pop == _capacity)
            return false;
        alloc_trait::construct(_allocator, get(push), std::forward<Args>(args)...);
        _pusher.store(push + 1, std::memory_order_release);
        return true;
    }

    std::optional<T> pop()
    {
        std::size_t pop = _popper.load(std::memory_order_relaxed);
        std::size_t push = _pusher.load(std::memory_order_acquire);
        if (pop == push)
            return std::nullopt;
        T element = std::move(*get(pop));
        alloc_trait::destroy(_allocator, get(pop));
        _popper.store(pop + 1, std::memory_order_release);
        return std::move(element);
    }

    bool isEmpty() const
    {
        std::size_t pop = _popper.load(std::memory_order_acquire);
        std::size_t push = _pusher.load(std::memory_order_acquire);
        return push == pop;
    }

    bool isFull() const
    {
        std::size_t push = _pusher.load(std::memory_order_acquire);
        std::size_t pop = _popper.load(std::memory_order_acquire);
        return push - pop == _capacity;
    }

    std::size_t size() const
    {
        std::size_t pop = _popper.load(std::memory_order_acquire);
        std::size_t push = _pusher.load(std::memory_order_acquire);
        return push - pop;
    }

    void reset()
    {
        std::size_t pop = _popper.load(std::memory_order_relaxed);
        std::size_t push = _pusher.load(std::memory_order_relaxed);
        for (; pop != push; ++pop)
            alloc_trait::destroy(_allocator, get(pop));
        _pusher.store(0, std::memory_order_relaxed);
        _popper.store(0, std::memory_order_relaxed);
    }

  private:
    T *get(std::size_t position) noexcept
    {
        return &_data[position % _capacity];
    }

    Allocator _allocator;
    const std::size_t _capacity;
    T *_data;
    alignas(64) std::atomic<std::size_t> _pusher{0};
    alignas(64) std::atomic<std::size_t> _popper{0};
};
