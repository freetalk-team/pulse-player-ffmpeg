FROM debian:13

RUN apt-get update && apt-get install -y \
    build-essential \
    nodejs \
    npm \
    nasm \
	cmake \
    ninja-build \
    zlib1g-dev \
    libzstd-dev \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

CMD ["/bin/bash"]
