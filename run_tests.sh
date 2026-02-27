cmake -S . -B build-gcc \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_CXX_COMPILER=g++
cmake --build build-gcc
cd build-gcc && ctest --output-on-failure
