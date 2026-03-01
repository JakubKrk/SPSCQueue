#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

#include "message.h"
#include "spscringbuffer.h"

TEST(SpscRingBufferThreaded, AllItemsDeliveredInOrder)
{
    constexpr uint64_t N = 100'000;
    SpscRingBuffer<Message> buf{1024};

    std::thread producer([&] {
        for (uint64_t i = 0; i < N; ++i)
        {
            Message msg{"msg", i};
            while (!buf.try_push(std::move(msg)))
            {
            }
        }
    });

    std::thread consumer([&] {
        for (uint64_t i = 0; i < N; ++i)
        {
            std::optional<Message> msg;
            while (!(msg = buf.pop()).has_value())
            {
            }
            EXPECT_EQ(msg->_value, i);
        }
    });

    producer.join();
    consumer.join();
}

TEST(SpscRingBufferThreaded, AllItemsDeliveredInOrderSmallCapacity)
{
    constexpr uint64_t N = 10000;
    SpscRingBuffer<Message> buf{8};
    std::vector<uint64_t> received;
    received.reserve(N);

    std::thread producer([&] {
        for (uint64_t i = 0; i < N; ++i)
        {
            Message msg{"bp", i};
            while (!buf.try_push(std::move(msg)))
            {
            }
        }
    });

    std::thread consumer([&] {
        for (uint64_t i = 0; i < N; ++i)
        {
            std::optional<Message> msg;
            while (!(msg = buf.pop()).has_value())
            {
            }
            received.push_back(msg->_value);
        }
    });

    producer.join();
    consumer.join();

    ASSERT_EQ(received.size(), N);
    for (uint64_t i = 0; i < N; ++i)
        EXPECT_EQ(received[i], i);
}

TEST(SpscRingBufferThreaded, ConsumerStartsEarly)
{
    constexpr uint64_t N = 50000;
    SpscRingBuffer<Message> buf{512};
    std::atomic<bool> consumer_ready{false};

    std::thread consumer([&] {
        consumer_ready.store(true, std::memory_order_release);
        for (uint64_t i = 0; i < N; ++i)
        {
            std::optional<Message> msg;
            while (!(msg = buf.pop()).has_value())
            {
            }
            EXPECT_EQ(msg->_value, i);
        }
    });

    while (!consumer_ready.load(std::memory_order_acquire))
    {
    }

    std::thread producer([&] {
        for (uint64_t i = 0; i < N; ++i)
        {
            Message msg{"s", i};
            while (!buf.try_push(std::move(msg)))
            {
            }
        }
    });

    consumer.join();
    producer.join();
}

TEST(SpscRingBufferThreaded, PushWaitPopWaitOrdering)
{
    constexpr uint64_t N = 100000;
    SpscRingBuffer<Message> buf{1024};

    std::thread producer([&] {
        for (uint64_t i = 0; i < N; ++i)
        {
            Message msg{"msg", i};
            buf.push_wait(std::move(msg));
        }
    });

    std::thread consumer([&] {
        for (uint64_t i = 0; i < N; ++i)
        {
            Message msg = buf.pop_wait();
            EXPECT_EQ(msg._value, i);
        }
    });

    producer.join();
    consumer.join();
}

TEST(SpscRingBufferThreaded, PushWaitBlocksUntilSpaceAvailable)
{
    constexpr std::size_t CAP = 4;
    SpscRingBuffer<Message> buf{CAP};

    for (uint64_t i = 0; i < CAP; ++i)
        ASSERT_TRUE(buf.try_push(Message{"x", i}));

    std::thread producer([&] {
        Message msg{"extra", CAP};
        buf.push_wait(std::move(msg));
    });

    std::thread consumer([&] {
        for (uint64_t i = 0; i < CAP + 1; ++i)
        {
            Message msg = buf.pop_wait();
            EXPECT_EQ(msg._value, i);
        }
    });

    producer.join();
    consumer.join();
}
