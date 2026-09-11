FROM debian:13-slim

RUN apt-get update && apt-get install -y \
    build-essential \
    nodejs \
    npm \
	cmake \
    ninja-build \
    zlib1g-dev \
    libzstd-dev \
    libssl-dev \
    gdb \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

CMD ["/bin/bash"]
