FROM ubuntu-dev-mingw-llvm:latest

COPY toolchains/windows-mingw-aarch64.cmake /usr/share/cmake/win-mingw-arm64.toolchain.cmake
COPY toolchains/windows-mingw-x64.cmake /usr/share/cmake/win-mingw-x64.toolchain.cmake

COPY zstd /usr/local/src/zstd
COPY zlib /usr/local/src/zlib
COPY openssl /usr/local/src/openssl

RUN mkdir -p /build/zstd
RUN cmake -S /usr/local/src/zstd -B /build/zstd -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=/usr/share/cmake/win-mingw-x64.toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DZSTD_BUILD_SHARED=OFF \
    -DZSTD_BUILD_STATIC=ON \
    -DZSTD_BUILD_PROGRAMS=OFF \
    -DZSTD_BUILD_TESTS=OFF \
    -DZSTD_BUILD_CONTRIB=OFF
RUN cmake --build /build/zstd
RUN cmake --install /build/zstd
RUN rm -rf /usr/local/src/zstd
RUN rm -rf /build/zstd

RUN mkdir -p /build/zlib
RUN cmake -S /usr/local/src/zlib -B /build/zlib -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=/usr/share/cmake/win-mingw-x64.toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DZLIB_BUILD_STATIC=ON \
    -DZLIB_BUILD_SHARED=OFF \
    -DZLIB_BUILD_TESTING=OFF 
RUN cmake --build /build/zlib
RUN cmake --install /build/zlib
RUN rm -rf /usr/local/src/zlib
RUN rm -rf /build/zlib

RUN rm -rf /build

RUN cd /usr/local/src/openssl && ./Configure mingw64 --cross-compile-prefix=x86_64-w64-mingw32- no-shared no-tests no-apps --with-zlib-include=/usr/local/include --with-zlib-lib=/usr/local/lib --prefix=/usr/local && make && make install_sw
RUN rm -rf /usr/local/src/openssl

WORKDIR /workspace

CMD ["/bin/bash"]

