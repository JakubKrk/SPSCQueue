FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    clang lld llvm libclang-rt-dev \
    cmake build-essential ninja-build gcc g++ gdb git ca-certificates \
 && rm -rf /var/lib/apt/lists/*
WORKDIR /workspace
CMD ["bash"]
