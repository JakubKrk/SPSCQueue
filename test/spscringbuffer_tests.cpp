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
    ASSERT_TRUE(buf.try_push(Message{"hello", 1}));
    auto val = buf.pop();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val->_title, "hello");
    EXPECT_EQ(val->_value, 1U);
}

TEST(SpscRingBuffer, FifoOrdering)
{
    SpscRingBuffer<Message> buf{8};
    ASSERT_TRUE(buf.try_push(Message{"first", 1}));
    ASSERT_TRUE(buf.try_push(Message{"second", 2}));
    ASSERT_TRUE(buf.try_push(Message{"third", 3}));
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
        ASSERT_TRUE(buf.try_push(Message{"msg", i}));
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
    ASSERT_TRUE(buf.try_push(Message{"a", 1}));
    ASSERT_TRUE(buf.try_push(Message{"b", 2}));
    ASSERT_TRUE(buf.try_push(Message{"c", 3}));
    ASSERT_TRUE(buf.try_push(Message{"d", 4}));
    EXPECT_EQ(buf.pop()->_value, 1U);
    EXPECT_EQ(buf.pop()->_value, 2U);
    ASSERT_TRUE(buf.try_push(Message{"e", 5}));
    ASSERT_TRUE(buf.try_push(Message{"f", 6}));
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
        ASSERT_TRUE(buf.try_push(Message{"x", round * 10 + 1}));
        ASSERT_TRUE(buf.try_push(Message{"y", round * 10 + 2}));
        EXPECT_EQ(buf.pop()->_value, round * 10 + 1);
        EXPECT_EQ(buf.pop()->_value, round * 10 + 2);
    }
    EXPECT_EQ(buf.pop(), std::nullopt);
}

TEST(SpscRingBuffer, InterleavedPushPop)
{
    SpscRingBuffer<Message> buf{4};
    ASSERT_TRUE(buf.try_push(Message{"a", 1}));
    EXPECT_EQ(buf.pop()->_value, 1U);
    ASSERT_TRUE(buf.try_push(Message{"b", 2}));
    ASSERT_TRUE(buf.try_push(Message{"c", 3}));
    EXPECT_EQ(buf.pop()->_value, 2U);
    ASSERT_TRUE(buf.try_push(Message{"d", 4}));
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
    ASSERT_TRUE(buf.try_push(Message{"a", 1}));
    EXPECT_FALSE(buf.isEmpty());
    EXPECT_EQ(buf.size(), 1U);
}

TEST(SpscRingBuffer, IsEmptyTrueAfterDrain)
{
    SpscRingBuffer<Message> buf{4};
    ASSERT_TRUE(buf.try_push(Message{"a", 1}));
    buf.pop();
    EXPECT_TRUE(buf.isEmpty());
    EXPECT_EQ(buf.size(), 0U);
}

TEST(SpscRingBuffer, IsFullWhenAtCapacity)
{
    SpscRingBuffer<Message> buf{4};
    EXPECT_FALSE(buf.isFull());
    ASSERT_TRUE(buf.try_push(Message{"a", 1}));
    ASSERT_TRUE(buf.try_push(Message{"b", 2}));
    ASSERT_TRUE(buf.try_push(Message{"c", 3}));
    EXPECT_FALSE(buf.isFull());
    ASSERT_TRUE(buf.try_push(Message{"d", 4}));
    EXPECT_TRUE(buf.isFull());
}

TEST(SpscRingBuffer, IsFullFalseAfterPop)
{
    SpscRingBuffer<Message> buf{4};
    ASSERT_TRUE(buf.try_push(Message{"a", 1}));
    ASSERT_TRUE(buf.try_push(Message{"b", 2}));
    ASSERT_TRUE(buf.try_push(Message{"c", 3}));
    ASSERT_TRUE(buf.try_push(Message{"d", 4}));
    buf.pop();
    EXPECT_FALSE(buf.isFull());
}

TEST(SpscRingBuffer, SizeTracksElements)
{
    SpscRingBuffer<Message> buf{8};
    for (uint64_t i = 1; i <= 4; ++i)
    {
        ASSERT_TRUE(buf.try_push(Message{"m", i}));
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
    ASSERT_TRUE(buf.try_push(Message{"a", 1}));
    ASSERT_TRUE(buf.try_push(Message{"b", 2}));
    buf.reset();
    EXPECT_TRUE(buf.isEmpty());
    EXPECT_EQ(buf.size(), 0U);
    EXPECT_FALSE(buf.isFull());

    EXPECT_EQ(buf.pop(), std::nullopt);
}

TEST(SpscRingBuffer, ResetAllowsReuse)
{
    SpscRingBuffer<Message> buf{4};
    ASSERT_TRUE(buf.try_push(Message{"a", 1}));
    ASSERT_TRUE(buf.try_push(Message{"b", 2}));
    buf.reset();
    ASSERT_TRUE(buf.try_push(Message{"c", 3}));
    ASSERT_TRUE(buf.try_push(Message{"d", 4}));
    EXPECT_EQ(buf.size(), 2U);
    EXPECT_EQ(buf.pop()->_value, 3U);
    EXPECT_EQ(buf.pop()->_value, 4U);
    EXPECT_EQ(buf.pop(), std::nullopt);
}

TEST(SpscRingBuffer, ResetAfterWrapAround)
{
    SpscRingBuffer<Message> buf{4};
    ASSERT_TRUE(buf.try_push(Message{"a", 1}));
    ASSERT_TRUE(buf.try_push(Message{"b", 2}));
    buf.pop();
    buf.pop();
    ASSERT_TRUE(buf.try_push(Message{"c", 3}));
    ASSERT_TRUE(buf.try_push(Message{"d", 4}));
    buf.reset();
    EXPECT_TRUE(buf.isEmpty());
    ASSERT_TRUE(buf.try_push(Message{"e", 5}));
    EXPECT_EQ(buf.pop()->_value, 5U);
}

TEST(SpscRingBuffer, PushFromRvalue)
{
    SpscRingBuffer<Message> buf{4};
    ASSERT_TRUE(buf.try_push(Message{"rvalue", 1}));
    EXPECT_EQ(buf.pop()->_title, "rvalue");
}

TEST(SpscRingBuffer, PushFromLvalue)
{
    SpscRingBuffer<Message> buf{4};
    Message msg{"lvalue", 2};
    ASSERT_TRUE(buf.try_push(std::move(msg)));
    EXPECT_EQ(buf.pop()->_title, "lvalue");
}

TEST(SpscRingBuffer, PushInPlace)
{
    SpscRingBuffer<Message> buf{4};
    ASSERT_TRUE(buf.try_push("in-place", 42U));
    auto val = buf.pop();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val->_title, "in-place");
    EXPECT_EQ(val->_value, 42U);
}

TEST(SpscRingBuffer, PushReturnsFalseWhenFull)
{
    SpscRingBuffer<Message> buf{2};
    EXPECT_TRUE(buf.try_push(Message{"a", 1}));
    EXPECT_TRUE(buf.try_push(Message{"b", 2}));
    EXPECT_FALSE(buf.try_push(Message{"c", 3}));
}

TEST(SpscRingBuffer, PushWhenFullDoesNotCorruptBuffer)
{
    SpscRingBuffer<Message> buf{2};
    ASSERT_TRUE(buf.try_push(Message{"a", 1}));
    ASSERT_TRUE(buf.try_push(Message{"b", 2}));
    EXPECT_FALSE(buf.try_push(Message{"c", 3}));
    EXPECT_EQ(buf.size(), 2U);
    EXPECT_EQ(buf.pop()->_title, "a");
    EXPECT_EQ(buf.pop()->_title, "b");
    EXPECT_EQ(buf.pop(), std::nullopt);
}

TEST(SpscRingBuffer, PushSucceedsAgainAfterPop)
{
    SpscRingBuffer<Message> buf{2};
    ASSERT_TRUE(buf.try_push(Message{"a", 1}));
    ASSERT_TRUE(buf.try_push(Message{"b", 2}));
    ASSERT_FALSE(buf.try_push(Message{"c", 3}));
    buf.pop();
    EXPECT_TRUE(buf.try_push(Message{"c", 3}));
    EXPECT_EQ(buf.pop()->_title, "b");
    EXPECT_EQ(buf.pop()->_title, "c");
}
