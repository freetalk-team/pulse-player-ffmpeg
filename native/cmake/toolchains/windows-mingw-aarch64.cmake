set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR ARM64)

set(CMAKE_C_COMPILER   /opt/llvm-mingw/bin/aarch64-w64-mingw32-clang)
set(CMAKE_CXX_COMPILER /opt/llvm-mingw/bin/aarch64-w64-mingw32-clang++)

set(CMAKE_RC_COMPILER /opt/llvm-mingw/bin/aarch64-w64-mingw32-windres)

set(CMAKE_AR      /opt/llvm-mingw/bin/aarch64-w64-mingw32-ar)
set(CMAKE_RANLIB  /opt/llvm-mingw/bin/aarch64-w64-mingw32-ranlib)
set(CMAKE_STRIP   /opt/llvm-mingw/bin/aarch64-w64-mingw32-strip)
set(CMAKE_NM      /opt/llvm-mingw/bin/aarch64-w64-mingw32-nm)
set(CMAKE_OBJCOPY /opt/llvm-mingw/bin/aarch64-w64-mingw32-objcopy)
set(CMAKE_OBJDUMP /opt/llvm-mingw/bin/aarch64-w64-mingw32-objdump)

# LLVM-MinGW sysroot
set(CMAKE_SYSROOT /opt/llvm-mingw/aarch64-w64-mingw32)

# Don't search the host system for libraries/includes/packages.
set(CMAKE_FIND_ROOT_PATH /opt/llvm-mingw/aarch64-w64-mingw32)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
