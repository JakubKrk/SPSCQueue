#pragma once
#include <array>
#include <cstdint>
#include <string>

// Non trivially construcable class
class Message
{
  public:
    Message() = delete;
    explicit Message(std::string title, uint64_t value) : _title(title), _value(value)
    {
        _data.fill(1);
    }

    ~Message() = default;
    Message(const Message &) = delete;
    Message &operator=(const Message &) = delete;

    Message(Message &&other) noexcept = default;
    Message &operator=(Message &&other) noexcept = default;

    std::string _title;
    std::uint64_t _value;
    std::array<uint16_t, 10> _data;
};
