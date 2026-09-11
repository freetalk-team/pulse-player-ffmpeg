cmake_minimum_required (VERSION 3.28)

project(PulsePlayerFFMpeg 
    LANGUAGES C CXX ASM
    VERSION 1.0.0
    HOMEPAGE_URL "https://www.sipme.io"
    DESCRIPTION "FFMpeg wrapper for NodeJS"
)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

option (BUILD_TESTS "Build tests" OFF)

# find_package(NodeJS REQUIRED)
# find_package(ZLIB REQUIRED)

if (MINGW)
	
	# Find the host clang in llvm-mingw
	find_program(HOST_CC
		NAMES gcc
		PATHS /usr/bin
		NO_DEFAULT_PATH
	)

	if(NOT HOST_CC)
		# Fall back to system clang
		find_program(HOST_CC clang)
		if(NOT HOST_CC)
			message(FATAL_ERROR "No host compiler found for building generator")
		endif()
	endif()

	message(STATUS "Using host compiler: ${HOST_CC}")


	#set(ZLIB_ROOT "/usr/local" CACHE PATH "ZLIB installation")

	add_library(ZLIB::ZLIB STATIC IMPORTED)

	set_target_properties(ZLIB::ZLIB PROPERTIES
		IMPORTED_LOCATION "/usr/local/lib/libzs.a"
		INTERFACE_INCLUDE_DIRECTORIES "/usr/local/include"
	)

	#set(ZLIB_ROOT /usr/local)
	#set(ZLIB_LIBRARY /usr/local/lib/libzs.a)
	#set(ZLIB_INCLUDE_DIR /usr/local/include)

	if (CMAKE_SYSTEM_PROCESSOR STREQUAL "ARM64")
		set (SSL_LIB_DIR "/usr/local/lib-arm64")
	else ()
		set (SSL_LIB_DIR "/usr/local/lib64")
	endif ()

	add_library(OpenSSL::Crypto STATIC IMPORTED)
	set_target_properties(OpenSSL::Crypto PROPERTIES
		IMPORTED_LOCATION "${SSL_LIB_DIR}/libcrypto.a"
		INTERFACE_INCLUDE_DIRECTORIES "/usr/local/include"
	)

	add_library(OpenSSL::SSL STATIC IMPORTED)
	set_target_properties(OpenSSL::SSL PROPERTIES
		IMPORTED_LOCATION "${SSL_LIB_DIR}/libssl.a"
		INTERFACE_INCLUDE_DIRECTORIES "/usr/local/include"
	)

	#set(OPENSSL_ROOT_DIR /usr/local)
	#set(OPENSSL_CRYPTO_LIBRARY "/usr/lib64/libcrypto.a")
	#set(OPENSSL_SSL_LIBRARY "/usr/lib64/libssl.a")
		#set(OPENSSL_INCLUDE_DIR "/usr/include")

	add_library(zstd::zstd STATIC IMPORTED)

	set_target_properties(zstd::zstd PROPERTIES
		IMPORTED_LOCATION "/usr/local/lib/libzstd.a"
		INTERFACE_INCLUDE_DIRECTORIES "/usr/local/include"
	)
else ()

	set(ZLIB_USE_STATIC_LIBS TRUE)
	find_package(ZLIB REQUIRED)

	set(OPENSSL_USE_STATIC_LIBS TRUE)
	find_package(OpenSSL REQUIRED)

	set(ZSTD_USE_STATIC_LIBS TRUE)
	find_package(ZSTD REQUIRED)

endif ()


if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set (LINUX TRUE)
endif ()

if (LINUX)
    set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
elseif (MSVC)
    set(CMAKE_ASM_NASM_OBJECT_FORMAT win64)
endif ()

include (cmake/Detect.cmake)
include (cmake/Utils.cmake)


if (ARCH_X86)
    enable_language(ASM_NASM)
endif ()

add_subdirectory (libs/dav1d)
# add_subdirectory (libs/zlib)
# add_subdirectory (libs/boringssl)
add_subdirectory (libs/ffmpeg)

add_subdirectory (src)


macro (add_test NAME)
    add_executable (${NAME} ${ARGN})
    target_link_libraries (${NAME} PRIVATE 
        ffmpeg
        "$<LINK_GROUP:RESCAN,dav1d,dav1d_bitdepth_8>"
        OpenSSL::SSL
        OpenSSL::Crypto
		zstd::zstd
        ZLIB::ZLIB
    )
endmacro()

if (BUILD_TESTS)
    add_test (parse_test tests/parse_test.cpp)
    add_test (thumb_test tests/thumb_test.cpp)
endif ()