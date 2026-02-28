#pragma once
#include <cstdint>
#include <string>

// Non trivially construcable class
class Message
{
  public:
    Message() = delete;
    explicit Message(std::string title, uint64_t value) : _title(title), _value(value)
    {
        _data = new uint16_t[10];
    }

    Message(const Message &) = delete;
    Message &operator=(const Message &) = delete;

    Message(Message &&other) noexcept : _title(std::move(other._title)), _value(other._value), _data(other._data)
    {
        other._data = nullptr;
    }

    Message &operator=(Message &&other) noexcept
    {
        if (this != &other)
        {
            delete[] _data;
            _title = std::move(other._title);
            _value = other._value;
            _data = other._data;
            other._data = nullptr;
        }
        return *this;
    }

    ~Message()
    {
        delete[] _data;
    }

    std::string _title;
    std::uint64_t _value;
    uint16_t *_data;
};
