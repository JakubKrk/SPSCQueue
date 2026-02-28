#include <gtest/gtest.h>

#include "message.h"
#include "spscringbuffer.h"

TEST(SpscRingBuffer, PopFromEmptyReturnsNullopt)
{
    SpscRingBuffer<Message> buf{4};
    EXPECT_EQ(buf.pop(), std::nullopt);
}

TEST(SpscRingBuffer, PopAfterPushReturnsValue)
{
    SpscRingBuffer<Message> buf{4};
    buf.push(Message{"hello", 1});
    auto val = buf.pop();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val->_title, "hello");
    EXPECT_EQ(val->_value, 1U);
}

TEST(SpscRingBuffer, FifoOrdering)
{
    SpscRingBuffer<Message> buf{8};
    buf.push(Message{"first", 1});
    buf.push(Message{"second", 2});
    buf.push(Message{"third", 3});
    EXPECT_EQ(buf.pop()->_title, "first");
    EXPECT_EQ(buf.pop()->_title, "second");
    EXPECT_EQ(buf.pop()->_title, "third");
    EXPECT_EQ(buf.pop(), std::nullopt);
}

TEST(SpscRingBuffer, FillThenDrain)
{
    constexpr std::size_t cap = 8;
    SpscRingBuffer<Message> buf{cap};
    for (uint64_t i = 0; i < cap; ++i)
        buf.push(Message{"msg", i});
    for (uint64_t i = 0; i < cap; ++i)
    {
        auto val = buf.pop();
        ASSERT_TRUE(val.has_value());
        EXPECT_EQ(val->_value, i);
    }
    EXPECT_EQ(buf.pop(), std::nullopt);
}

TEST(SpscRingBuffer, WrapAround)
{
    SpscRingBuffer<Message> buf{4};
    buf.push(Message{"a", 1});
    buf.push(Message{"b", 2});
    buf.push(Message{"c", 3});
    buf.push(Message{"d", 4});
    EXPECT_EQ(buf.pop()->_value, 1U);
    EXPECT_EQ(buf.pop()->_value, 2U);
    buf.push(Message{"e", 5});
    buf.push(Message{"f", 6});
    EXPECT_EQ(buf.pop()->_value, 3U);
    EXPECT_EQ(buf.pop()->_value, 4U);
    EXPECT_EQ(buf.pop()->_value, 5U);
    EXPECT_EQ(buf.pop()->_value, 6U);
    EXPECT_EQ(buf.pop(), std::nullopt);
}

TEST(SpscRingBuffer, MultipleWrapArounds)
{
    SpscRingBuffer<Message> buf{4};
    for (uint64_t round = 0; round < 4; ++round)
    {
        buf.push(Message{"x", round * 10 + 1});
        buf.push(Message{"y", round * 10 + 2});
        EXPECT_EQ(buf.pop()->_value, round * 10 + 1);
        EXPECT_EQ(buf.pop()->_value, round * 10 + 2);
    }
    EXPECT_EQ(buf.pop(), std::nullopt);
}

TEST(SpscRingBuffer, InterleavedPushPop)
{
    SpscRingBuffer<Message> buf{4};
    buf.push(Message{"a", 1});
    EXPECT_EQ(buf.pop()->_value, 1U);
    buf.push(Message{"b", 2});
    buf.push(Message{"c", 3});
    EXPECT_EQ(buf.pop()->_value, 2U);
    buf.push(Message{"d", 4});
    EXPECT_EQ(buf.pop()->_value, 3U);
    EXPECT_EQ(buf.pop()->_value, 4U);
    EXPECT_EQ(buf.pop(), std::nullopt);
}

TEST(SpscRingBuffer, IsEmptyOnNewBuffer)
{
    SpscRingBuffer<Message> buf{4};
    EXPECT_TRUE(buf.isEmpty());
    EXPECT_EQ(buf.size(), 0U);
}

TEST(SpscRingBuffer, IsEmptyFalseAfterPush)
{
    SpscRingBuffer<Message> buf{4};
    buf.push(Message{"a", 1});
    EXPECT_FALSE(buf.isEmpty());
    EXPECT_EQ(buf.size(), 1U);
}

TEST(SpscRingBuffer, IsEmptyTrueAfterDrain)
{
    SpscRingBuffer<Message> buf{4};
    buf.push(Message{"a", 1});
    buf.pop();
    EXPECT_TRUE(buf.isEmpty());
    EXPECT_EQ(buf.size(), 0U);
}

TEST(SpscRingBuffer, IsFullWhenAtCapacity)
{
    SpscRingBuffer<Message> buf{4};
    EXPECT_FALSE(buf.isFull());
    buf.push(Message{"a", 1});
    buf.push(Message{"b", 2});
    buf.push(Message{"c", 3});
    EXPECT_FALSE(buf.isFull());
    buf.push(Message{"d", 4});
    EXPECT_TRUE(buf.isFull());
}

TEST(SpscRingBuffer, IsFullFalseAfterPop)
{
    SpscRingBuffer<Message> buf{4};
    buf.push(Message{"a", 1});
    buf.push(Message{"b", 2});
    buf.push(Message{"c", 3});
    buf.push(Message{"d", 4});
    buf.pop();
    EXPECT_FALSE(buf.isFull());
}

TEST(SpscRingBuffer, SizeTracksElements)
{
    SpscRingBuffer<Message> buf{8};
    for (uint64_t i = 1; i <= 4; ++i)
    {
        buf.push(Message{"m", i});
        EXPECT_EQ(buf.size(), i);
    }
    for (uint64_t i = 3; i > 0; --i)
    {
        buf.pop();
        EXPECT_EQ(buf.size(), i);
    }
}

TEST(SpscRingBuffer, ResetEmptiesBuffer)
{
    SpscRingBuffer<Message> buf{4};
    buf.push(Message{"a", 1});
    buf.push(Message{"b", 2});
    buf.reset();
    EXPECT_TRUE(buf.isEmpty());
    EXPECT_EQ(buf.size(), 0U);
    EXPECT_FALSE(buf.isFull());
    EXPECT_EQ(buf.pop(), std::nullopt);
}

TEST(SpscRingBuffer, ResetAllowsReuse)
{
    SpscRingBuffer<Message> buf{4};
    buf.push(Message{"a", 1});
    buf.push(Message{"b", 2});
    buf.reset();
    buf.push(Message{"c", 3});
    buf.push(Message{"d", 4});
    EXPECT_EQ(buf.size(), 2U);
    EXPECT_EQ(buf.pop()->_value, 3U);
    EXPECT_EQ(buf.pop()->_value, 4U);
    EXPECT_EQ(buf.pop(), std::nullopt);
}

TEST(SpscRingBuffer, ResetAfterWrapAround)
{
    SpscRingBuffer<Message> buf{4};
    buf.push(Message{"a", 1});
    buf.push(Message{"b", 2});
    buf.pop();
    buf.pop();
    buf.push(Message{"c", 3});
    buf.push(Message{"d", 4});
    buf.reset();
    EXPECT_TRUE(buf.isEmpty());
    buf.push(Message{"e", 5});
    EXPECT_EQ(buf.pop()->_value, 5U);
}
