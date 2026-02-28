#include <cassert>
#include <cstdio>
#include <cstring>

#include "message.h"
#include "spscringbuffer.h"

#define RUN_TEST(fn)                                                                                                   \
    do                                                                                                                 \
    {                                                                                                                  \
        std::printf("[RUN ] " #fn "\n");                                                                               \
        fn();                                                                                                          \
        std::printf("[PASS] " #fn "\n");                                                                               \
    } while (0)

static void test_push_pop_fields_initialized()
{
    SpscRingBuffer<Message> buf{8};
    assert(buf.try_push(Message{"hello", 42}));
    auto msg = buf.pop();
    assert(msg.has_value());
    assert(msg->_title == "hello");
    assert(msg->_value == 42U);
    assert(msg->_data != nullptr);
    msg->_data[0] = 1;
    assert(msg->_data[0] == 1);
}

static void test_wrap_around_reads_initialized_memory()
{
    SpscRingBuffer<Message> buf{4};
    assert(buf.try_push(Message{"a", 1}));
    assert(buf.try_push(Message{"b", 2}));
    assert(buf.try_push(Message{"c", 3}));
    assert(buf.try_push(Message{"d", 4}));

    auto a = buf.pop();
    auto b = buf.pop();
    assert(a->_value == 1U);
    assert(b->_value == 2U);

    assert(buf.try_push(Message{"e", 5}));
    assert(buf.try_push(Message{"f", 6}));

    assert(buf.pop()->_value == 3U);
    assert(buf.pop()->_value == 4U);

    auto e = buf.pop();
    auto f = buf.pop();
    assert(e->_value == 5U);
    assert(f->_value == 6U);
    assert(e->_data != nullptr);
    assert(f->_data != nullptr);
}

static void test_reset_then_reuse_initialized()
{
    SpscRingBuffer<Message> buf{4};
    assert(buf.try_push(Message{"x", 10}));
    assert(buf.try_push(Message{"y", 20}));
    buf.reset();

    assert(buf.isEmpty());
    assert(buf.size() == 0U);

    assert(buf.try_push(Message{"new", 99}));
    auto msg = buf.pop();
    assert(msg.has_value());
    assert(msg->_title == "new");
    assert(msg->_value == 99U);
    assert(msg->_data != nullptr);
}

static void test_reset_after_wrap_around()
{
    SpscRingBuffer<Message> buf{4};
    assert(buf.try_push(Message{"a", 1}));
    assert(buf.try_push(Message{"b", 2}));
    buf.pop();
    buf.pop();
    assert(buf.try_push(Message{"c", 3}));
    assert(buf.try_push(Message{"d", 4}));
    buf.reset();

    assert(buf.try_push(Message{"e", 5}));
    auto msg = buf.pop();
    assert(msg->_value == 5U);
    assert(msg->_data != nullptr);
    assert(buf.isEmpty());
}

static void test_state_queries_initialized()
{
    SpscRingBuffer<Message> buf{4};
    assert(buf.isEmpty());
    assert(!buf.isFull());
    assert(buf.size() == 0U);

    assert(buf.try_push(Message{"m", 1}));
    assert(!buf.isEmpty());
    assert(!buf.isFull());
    assert(buf.size() == 1U);

    assert(buf.try_push(Message{"m", 2}));
    assert(buf.try_push(Message{"m", 3}));
    assert(buf.try_push(Message{"m", 4}));
    assert(buf.isFull());
    assert(buf.size() == 4U);

    buf.pop();
    assert(!buf.isFull());
    assert(buf.size() == 3U);
}

int main()
{
    RUN_TEST(test_push_pop_fields_initialized);
    RUN_TEST(test_wrap_around_reads_initialized_memory);
    RUN_TEST(test_reset_then_reuse_initialized);
    RUN_TEST(test_reset_after_wrap_around);
    RUN_TEST(test_state_queries_initialized);
    return 0;
}
