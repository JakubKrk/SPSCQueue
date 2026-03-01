#pragma once
#include <atomic>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <thread>
#include <type_traits>

template <typename T, typename Allocator = std::allocator<T>> class SpscRingBuffer
{
    static_assert(std::is_nothrow_move_constructible_v<T>, "T must be nothrow move constructible");
    static_assert(std::is_nothrow_destructible_v<T>, "T must be nothrow destructible");
    static_assert(std::atomic<std::size_t>::is_always_lock_free);

  public:
    using alloc_trait = std::allocator_traits<Allocator>;

    SpscRingBuffer() = delete;
    SpscRingBuffer(const SpscRingBuffer &) = delete;
    SpscRingBuffer &operator=(const SpscRingBuffer &) = delete;
    SpscRingBuffer(SpscRingBuffer &&) = delete;
    SpscRingBuffer &operator=(SpscRingBuffer &&) = delete;

    explicit SpscRingBuffer(std::size_t capacity, const Allocator &allocator = Allocator{})
        : _capacity(validate(capacity)), _mask(_capacity - 1), _allocator(allocator),
          _data(alloc_trait::allocate(_allocator, _capacity))
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

    template <typename... Args>
    [[nodiscard]] bool try_push(Args &&...args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        std::size_t push = _pusher.load(std::memory_order_relaxed);
        std::size_t pop = _popper.load(std::memory_order_acquire);
        if (push - pop == _capacity)
            return false;
        alloc_trait::construct(_allocator, get(push), std::forward<Args>(args)...);
        _pusher.store(push + 1, std::memory_order_release);
        return true;
    }

    template <typename... Args> void push_wait(Args &&...args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        while (!try_push(std::forward<Args>(args)...))
            std::this_thread::yield();
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

    T pop_wait()
    {
        std::optional<T> result;
        while (!(result = pop()).has_value())
            std::this_thread::yield();
        return std::move(*result);
    }

    bool is_empty() const
    {
        std::size_t pop = _popper.load(std::memory_order_acquire);
        std::size_t push = _pusher.load(std::memory_order_acquire);
        return push == pop;
    }

    bool is_full() const
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
    static std::size_t validate(std::size_t capacity)
    {
        if (capacity == 0 || (capacity & (capacity - 1)) != 0)
            throw std::invalid_argument("Capacity must be a non-zero power of two");
        return capacity;
    }

    T *get(std::size_t position) noexcept
    {
        return &_data[position & _mask];
    }

    Allocator _allocator;
    const std::size_t _capacity;
    const std::size_t _mask;
    T *_data;
    alignas(64) std::atomic<std::size_t> _pusher{0};
    alignas(64) std::atomic<std::size_t> _popper{0};
};
