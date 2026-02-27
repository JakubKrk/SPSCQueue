#include <gtest/gtest.h>

#include "spscringbuffer.h"

class SpscRingBufferTest : public ::testing::Test
{
  protected:
    SpscRingBuffer buffer;
};

TEST_F(SpscRingBufferTest, Push)
{
    EXPECT_EQ(buffer.push(), true);
}

TEST_F(SpscRingBufferTest, Pop)
{
    EXPECT_EQ(buffer.pop(), false);
}
