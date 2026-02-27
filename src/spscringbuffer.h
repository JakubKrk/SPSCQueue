#pragma once

class SpscRingBuffer
{
  public:
    SpscRingBuffer() = default;

    bool push();
    bool pop();
};
