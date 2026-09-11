
project (ffmpeg)

# if (LINUX)
#     find_package(PkgConfig REQUIRED)
#     pkg_check_modules(DAV1D REQUIRED dav1d)
# endif ()


include (Config.cmake)
include (ConfigComponents.cmake)

if (LINUX
   AND HAVE_LINUX_PERF_EVENT_H
   AND HAVE_SYS_SYSCALL_H
   AND HAVE_NR_PERF_EVENT_OPEN)
    set(CONFIG_LINUX_PERF 1)
else()
    set(CONFIG_LINUX_PERF 0)
endif()

configure_file(config.h.in config.h)
configure_file(config_components.h.in config_components.h)
configure_file(config_components.asm.in config_components.asm)


set(NASM_DEFINES)

if (CONFIG_PIC)
    list (APPEND NASM_DEFINES PIC)
endif ()

add_nasm_definitions(
    ARCH_X86_64
    ARCH_X86_32
    HAVE_ALIGNED_STACK
    HAVE_AVX_EXTERNAL
    HAVE_AVX2_EXTERNAL
    HAVE_FMA3_EXTERNAL
    HAVE_X86_SSE2AVX
    HAVE_XOP_EXTERNAL
    HAVE_AVX512ICL_EXTERNAL
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# get_directory_property(dirs INCLUDE_DIRECTORIES)
# message(STATUS "parent include dirs = ${dirs}")

add_definitions(-DHAVE_AV_CONFIG_H)

add_subdirectory(libavutil)
add_subdirectory(libavcodec)
add_subdirectory(libavformat)
add_subdirectory(libswscale)

add_library(ffmpeg STATIC
    $<TARGET_OBJECTS:avutil>
    $<TARGET_OBJECTS:avcodec>
    $<TARGET_OBJECTS:avformat>
    $<TARGET_OBJECTS:swscale>
)

target_include_directories(ffmpeg PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

if (CONFIG_PIC)
    message(STATUS "PIC enabled")

    set_target_properties(ffmpeg PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
endif ()

if (LINUX)
    target_compile_options(ffmpeg PRIVATE -fvisibility=hidden)
    # target_link_options(ffmpeg PRIVATE "-Wl,--no-undefined")
endif ()

