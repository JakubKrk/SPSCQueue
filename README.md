Simple implementation of single-consumer single-producer fixed size queue in C++, which
ensures thread-safe usage and robust performance.

Assumptions:
  + Single producer, single consumer
  + Thread-safe for push/pop operations
  + Fixed-size
  + Bounded ring buffer to store data
  + Lock-free - uses atomics operations which ensure happens-before contracts between          pushing and poping
  
To run tests:

For Windows/MacOS - build and run docker image

docker compose run dev

Inside docker container /workspace directory run script:

.scripts/run_tests.sh

To run sanitizers on tests suites:

./script/run_sanitizers.sh <-- TSan, ASan, Usan
./scripts/run_memtests.sh <-- MSan (added on another suite without GTest due to conflicts)

For Linux just run script directly

SPSC Ring Buffer API:

+ SpscRingBuffer(capacity) 
    constructor with capacity argument

+ bool try_push(args)
    tries to push element into queue. Returns true if operation is successful (queue not full) and false when operation is not successful. To use in spin loop with low latency.

+ void push_wait(args)
    tries to push element into queue. Yields thread if operation is unsuccessful and tries again. Bigger latenct than try_push, but lower CPU usage.

+ optional T pop()
    tries to pop element from the front of the queue. Returns std::optional if queue is empty. Return element if queue is not empty and operation is successful. To use in spin loop with low latency.

+ T pop_wait()
    tries to pop element from the front of the queue. Yields the thread and tries again if unsuccessful. Bigger latency than pop, but lower CPU usage.

+ bool is_full()
    check if queue if full.

+ bool is_empty()
    checks if queue is empty.

+ size_t size()
    return the size of the queue

+ void reset()
    clears the queue. Not thread-safe. Cannot be safely called if producer/consumer threads are pushing/poping.


Limitations:

+ Desctruction/reset of the queue must happen after producer/consumer threads finished the work to ensure no memory leak.

+ is_full, is_empty, size are only approximates

+ Not resizeable.

+ Capacity must be power of 2. To make AND operation in get() work correctly.

+ No logging/tracing for debug mode added.

+ For small T possible false sharing can happen. For assumed message type it doesn't happen.

