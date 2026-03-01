Implementation of single-consumer single-producer fixed size queue in C++, which
ensures thread-safe usage and robust performance.

Assumptions:
  - Single producer, single consumer
  - Thread-safe for push/pop operations
  - Fixed-size
  - Bounded ring buffer to store data
  - Lock-free - uses atomics operations which ensure happens-before contracts between          pushing and poping
  
To run tests:

For Windows/MacOS - build and run docker image

docker compose run dev

Inside docker container /workspace directory run script:

.scripts/run_tests.sh

To run sanitizers on tests suites:

./script/run_sanitizers.sh <-- TSan, ASan, Usan
./scripts/run_memtests.sh <-- MSan (added on another suite without GTest due to conflicts)

SPSC Ring Buffer API:

1. SpscRingBuffer(size_t capacity) 
    constructor with capacity argument

2. bool try_emplace(Arg&&... arg)
    tries to push element into queue. Works for lvalues, rvalues. Returns true if operation is successful (queue not full) and false when operation is not successful.

3. std::optional<T> pop()
    tries to pop element from the front of the queue. Returns std::optional if queue is empty. Return element if queue is not empty and operation is successful.

4. bool isFull()
    check if queue if full.

5. bool isEmpty()
    checks if queue is empty.

6. size_t size()
    return the size of the queue

7. void reset()
    clears the queue. Not thread-safe. Cannot be safely called if producer/consumer threads are pushing/poping.

Desctruction of the queue must happen after producer/consumer threads finished the work to ensure no memory leak.



