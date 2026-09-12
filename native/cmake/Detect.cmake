
include(TestBigEndian)
include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckCSourceCompiles)
include(CheckStructHasMember)

find_package(Threads)


# Architecture

set(ARCH_X86_64 0)
set(ARCH_X86 0)
set(ARCH_ARM 0)
set(ARCH_AARCH64 0)
set(ARCH_MIPS 0)
set(ARCH_PPC 0)
set(ARCH_RISCV 0)
set(ARCH_LOONGARCH 0)
set(ARCH_LOONGARCH64 0)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|x86_x64|AMD64)$")
    set(ARCH_X86_64 1)
    set(ARCH_X86_32 0)
    set(ARCH_X86 1)

    set (HAVE_INTRINSICS_SSE2 1)
    set (HAVE_INLINE_ASM 1)
    set (HAVE_FAST_UNALIGNED 1)
    set (HAVE_SIMD_ALIGN_32 1)
    set (HAVE_SIMD_ALIGN_64 1)
    set (HAVE_FAST_64BIT 1)
    set (HAVE_ALIGNED_STACK 1)
    set (HAVE_X86ASM 1)
    set (HAVE_INLINE_ASM 1)
    set (HAVE_X86_SSE2AVX 1)
    set (HAVE_MMX 1)
    set (HAVE_MMX_INLINE 1)
    set (HAVE_MM_EMPTY 0)
    set (HAVE_AVX_EXTERNAL 0)
    set (HAVE_AVX2_EXTERNAL 0)
    set (HAVE_AVX512_EXTERNAL 0)
    set (HAVE_AVX512ICL_EXTERNAL 0)
    set (HAVE_FMA3_EXTERNAL 0)
    set (HAVE_FMA4_EXTERNAL 0)
    set (HAVE_MMX_EXTERNAL 0)
    set (HAVE_MMXEXT_EXTERNAL 0)
    set (HAVE_SSE_EXTERNAL 0)
    set (HAVE_SSE2_EXTERNAL 0)
    set (HAVE_SSE3_EXTERNAL 0)
    set (HAVE_SSE4_EXTERNAL 0)
    set (HAVE_SSE42_EXTERNAL 0)
    set (HAVE_SSSE3_EXTERNAL 0)
    set (HAVE_XOP_EXTERNAL 0)


elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i[3-6]86|x86)$")
    set(ARCH_X86 1)
    set(ARCH_X86_32 1)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)$")
    set(ARCH_AARCH64 ON)
    set(HAVE_ARM64 1)
    set(HAVE_NEON 1)
    set(HAVE_ARMV8 1)
    set(HAVE_SVE 0)
    set(HAVE_SME 0)
    set(HAVE_FAST_64BIT 1)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
    set(ARCH_ARM ON)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "mips")
    set(ARCH_MIPS ON)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "ppc|powerpc")
    set(ARCH_PPC ON)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "riscv")
    set(ARCH_RISCV ON)
endif()

if(WIN32)

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        set(WINDOWS_ARCH "x64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64")
        set(WINDOWS_ARCH "arm64")
    endif()

endif()

if(Threads_FOUND)
    set(HAVE_THREADS 1)

    if (UNIX)
        set (HAVE_PTHREADS 1)
    elseif (WIN32)
        set (HAVE_W32THREADS 1)
    endif ()
else()
    set(HAVE_THREADS 0)
endif()

if(WIN32)
    set(HAVE_ALIGNED_MALLOC 1)
    set(HAVE_DOS_PATHS 1)
    set(HAVE_POSIX_MEMALIGN 0)
else()
    set(HAVE_ALIGNED_MALLOC 0)
    set(HAVE_DOS_PATHS 0)
    set(HAVE_POSIX_MEMALIGN 1)
endif()

# Features

check_include_file(malloc.h HAVE_MALLOC_H)
check_include_file(poll.h HAVE_POLL_H)
check_include_file(unistd.h HAVE_UNISTD_H)
check_include_file(io.h HAVE_IO_H)
check_include_file(pthread_np.h HAVE_PTHREAD_NP_H)
check_include_file(malloc.h HAVE_MALLOC_H)
check_include_file(sys/param.h HAVE_SYS_PARAM_H)
check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
check_include_file(sys/select.h HAVE_SYS_SELECT_H)
check_include_file(sys/soundcard.h HAVE_SYS_SOUNDCARD_H)
check_include_file(sys/time.h HAVE_SYS_TIME_H)
check_include_file(sys/un.h HAVE_SYS_UN_H)
check_include_file(sys/videoio.h HAVE_SYS_VIDEOIO_H)
check_include_file(net/udplite.h HAVE_UDPLITE_H)
check_include_file(dirent.h HAVE_DIRENT_H)
check_include_file(direct.h HAVE_DIRECT_H)
check_include_file(sys/time.h HAVE_SYS_TIME_H)
check_include_file(sys/select.h HAVE_SYS_SELECT_H)
check_include_file(sys/syscall.h HAVE_SYS_SYSCALL_H)
check_include_file(linux/perf_event.h HAVE_LINUX_PERF_EVENT_H)
check_include_file(linux/dma-buf.h HAVE_LINUX_DMA_BUF_H)
check_include_file(windows.h HAVE_WINDOWS_H)
check_include_file(winsock2.h HAVE_WINSOCK2_H)

# check_include_file("va/va_drm.h"   HAVE_VAAPI_DRM)
# check_include_file("va/va_x11.h"   HAVE_VAAPI_X11)
# check_include_file("va/va_win32.h" HAVE_VAAPI_WIN32)

# if(WIN32)
#     set(HAVE_VAAPI_DRM 0)
#     set(HAVE_VAAPI_X11 0)
#     set(HAVE_VAAPI_WIN32 0)   # if you actually build with VA-API on Windows
# elseif(UNIX AND NOT APPLE)
#     find_package(PkgConfig)

#     if(PkgConfig_FOUND)
#         pkg_check_modules(LIBVA_DRM QUIET libva-drm)
#         pkg_check_modules(LIBVA_X11 QUIET libva-x11)
#     endif()

#     set(HAVE_VAAPI_DRM 0)
#     set(HAVE_VAAPI_X11 0)
#     set(HAVE_VAAPI_WIN32 0)

#     if(LIBVA_DRM_FOUND)
#         set(HAVE_VAAPI_DRM 1)
#     endif()

#     if(LIBVA_X11_FOUND)
#         set(HAVE_VAAPI_X11 1)
#     endif()

#     target_include_directories(your_target PRIVATE
#         ${LIBDRM_INCLUDE_DIRS}
#         ${LIBVA_DRM_INCLUDE_DIRS}
#     )

#     target_link_libraries(your_target PRIVATE
#         ${LIBDRM_LIBRARIES}
#         ${LIBVA_DRM_LIBRARIES}
#     )
# else()
#     set(HAVE_VAAPI_DRM 0)
#     set(HAVE_VAAPI_X11 0)
#     set(HAVE_VAAPI_WIN32 0)
# endif()

set(HAVE_VAAPI_DRM 0)
set(HAVE_VAAPI_X11 0)
set(HAVE_VAAPI_WIN32 0)


include(CheckCSourceCompiles)

check_c_source_compiles("
#include <stdlib.h>
int main() {
    void *p;
    return posix_memalign(&p,16,128);
}
" HAVE_POSIX_MEMALIGN)

test_big_endian(HAVE_BIGENDIAN)

check_function_exists(getaddrinfo HAVE_GETADDRINFO)
check_function_exists(inet_aton HAVE_INET_ATON)
check_function_exists(ioctl HAVE_IOCTL)

set(CMAKE_REQUIRED_LIBRARIES m)

check_function_exists(strerror_r HAVE_STRERROR_R)
check_function_exists(atanf HAVE_ATANF)
check_function_exists(atan2f HAVE_ATAN2F)
check_function_exists(powf HAVE_POWF)
check_function_exists(cbrt HAVE_CBRT)
check_function_exists(cbrtf HAVE_CBRTF)
check_function_exists(copysign HAVE_COPYSIGN)
check_function_exists(cosf HAVE_COSF)
check_function_exists(erf HAVE_ERF)
check_function_exists(expf HAVE_EXPF)
check_function_exists(exp2 HAVE_EXP2)
check_function_exists(exp2f HAVE_EXP2F)
check_function_exists(isinf HAVE_ISINF)
check_function_exists(isnan HAVE_ISNAN)
#check_function_exists(isfinite HAVE_ISFINITE)
check_function_exists(hypot HAVE_HYPOT)
check_function_exists(ldexpf HAVE_LDEXPF)
check_function_exists(llrint HAVE_LLRINT)
check_function_exists(llrintf HAVE_LLRINTF)
check_function_exists(log2 HAVE_LOG2)
check_function_exists(log2f HAVE_LOG2F)
check_function_exists(log10f HAVE_LOG10F)
check_function_exists(sinf HAVE_SINF)
check_function_exists(rint HAVE_RINT)
check_function_exists(lrint HAVE_LRINT)
check_function_exists(lrintf HAVE_LRINTF)
check_function_exists(round HAVE_ROUND)
check_function_exists(roundf HAVE_ROUNDF)
check_function_exists(trunc HAVE_TRUNC)
check_function_exists(truncf HAVE_TRUNCF)

check_c_source_compiles("
#include <math.h>

int main(void)
{
    double x = 1.0;
    return isfinite(x) ? 0 : 1;
}
" HAVE_ISFINITE)

unset(CMAKE_REQUIRED_LIBRARIES)

if (UNIX)
    set(CMAKE_REQUIRED_INCLUDES
        /usr/include
    )

    check_symbol_exists(__NR_perf_event_open
        "sys/syscall.h"
        HAVE_NR_PERF_EVENT_OPEN)

    check_symbol_exists(closesocket
        "sys/socket.h"
        HAVE_CLOSESOCKET
    )

    check_c_source_compiles("
#include <sys/socket.h>

int main(void)
{
    socklen_t len;
    (void)len;
    return 0;
}
" HAVE_SOCKLEN_T)

    check_c_source_compiles("
#include <sys/poll.h>

int main(void)
{
    struct pollfd s;
    (void)s;
    return 0;
}
" HAVE_STRUCT_POLLFD)

    check_c_source_compiles("
#include <sys/socket.h>

int main(void)
{
struct sockaddr_storage s;
(void)s;
return 0;
}
	" HAVE_STRUCT_SOCKADDR_STORAGE)

    check_c_source_compiles("
#include <sys/socket.h>

int main(void)
{
    struct sockaddr_in6 s;
    (void)s;
    return 0;
}
" HAVE_STRUCT_SOCKADDR_IN6)

    check_c_source_compiles("
#include <sys/socket.h>
#include <netdb.h>

int main(void)
{
    struct addrinfo s;
    (void)s;
    return 0;
}
" HAVE_STRUCT_ADDRINFO)

    check_struct_has_member(
        "struct sockaddr"
        sa_len
        "sys/socket.h"
        HAVE_STRUCT_SOCKADDR_SA_LEN
    )

    unset(CMAKE_REQUIRED_INCLUDES)
elseif (WIN32)

	set(CMAKE_REQUIRED_LIBRARIES ws2_32)

      check_c_source_compiles("
#include <winsock2.h>

int main(void)
{
    struct pollfd s;
    (void)s;
    return 0;
}
" HAVE_STRUCT_POLLFD)

	check_c_source_compiles("
#include <winsock2.h>
#include <ws2tcpip.h>

int main(void)
{
    socklen_t len;
    (void)len;
    return 0;
}
	" HAVE_SOCKLEN_T)

	check_symbol_exists(
		closesocket
		"winsock2.h"
		HAVE_CLOSESOCKET
	)

	check_c_source_compiles("
#include <winsock2.h>

int main(void)
{
struct sockaddr_storage s;
(void)s;
return 0;
}
    " HAVE_STRUCT_SOCKADDR_STORAGE)

    check_c_source_compiles("
#include <winsock2.h>
#include <ws2tcpip.h>

int main(void)
{
    struct addrinfo s;
    (void)s;
    return 0;
}
" HAVE_STRUCT_ADDRINFO)

    check_struct_has_member(
        "struct sockaddr"
        sa_len
        "winsock2.h;ws2tcpip.h"
        HAVE_STRUCT_SOCKADDR_SA_LEN
    )

	unset(CMAKE_REQUIRED_LIBRARIES)

endif ()



# check_c_source_compiles("
# #include <linux/perf_event.h>
# #include <sys/syscall.h>
# #include <sys/ioctl.h>
# #include <unistd.h>

# int main(void)
# {
#     struct perf_event_attr attr;
#     attr.size = sizeof(attr);

#     syscall(__NR_perf_event_open,
#             &attr, 0, -1, -1, 0);

#     return 0;
# }
# " CONFIG_LINUX_PERF)
