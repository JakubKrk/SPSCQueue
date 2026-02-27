Implementation of single-consumer single-producer fixed size queue in C++, which
ensures thread-safe usage and robust performance.

Assumptions:
  - Single producer, single consumer
  - Thread-safe
  - Fixed-size

To run tests:

1. build and run docker image

docker compose run dev

2. inside docker container /workspace directory run script:

./run_tests.sh
