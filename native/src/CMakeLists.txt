option (ENABLE_LOG "Enable module logging" OFF)
option (FFMPEG_DEBUG "Enable FFMpeg debug" OFF)

set (FFMPEG ${CMAKE_PROJECT_SOURCE_DIR})

execute_process(
    COMMAND node -p "require('node-addon-api').include"
    OUTPUT_VARIABLE NODE_ADDON_API_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(STRIP "${NODE_ADDON_API_DIR}" NODE_ADDON_API_DIR)
string(REGEX REPLACE "^\"|\"$" "" NODE_ADDON_API_DIR "${NODE_ADDON_API_DIR}")

message(STATUS "NODE_ADDON_API_DIR=${NODE_ADDON_API_DIR}")

# execute_process(
#     COMMAND node -p "require('node-addon-api').include_dir"
#     OUTPUT_VARIABLE NODE_ADDON_API_INCLUDE_DIR
#     OUTPUT_STRIP_TRAILING_WHITESPACE
# )

# message(STATUS "NODE_ADDON_API_INCLUDE_DIR=${NODE_ADDON_API_INCLUDE_DIR}")

find_path(NODE_API_INCLUDE_DIR
    NAMES node_api.h
    NO_CMAKE_FIND_ROOT_PATH
    HINTS
        /usr/include/node
        /usr/local/include/node
        ENV NODE_INCLUDE_DIR
)

message(STATUS "NODE_API_INCLUDE_DIR=${NODE_API_INCLUDE_DIR}")


# Platform name
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PP_PLATFORM linux)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PP_PLATFORM win32)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(PP_PLATFORM darwin)
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# Architecture name
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|x86_x64|AMD64)$")
    set(PP_ARCH x64)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|ARM64)$")
    set(PP_ARCH arm64)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
    set(PP_ARCH arm)
else()
    set(PP_ARCH ${CMAKE_SYSTEM_PROCESSOR})
endif()

# set(PP_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/../build/${PP_PLATFORM}-${PP_ARCH}")
if (BUILD_PACKAGE)
    set(PP_OUTPUT_DIR "${PROJECT_SOURCE_DIR}/../packages/${PP_PLATFORM}-${PP_ARCH}")
else ()
    set (PP_OUTPUT_DIR ${CMAKE_BINARY_DIR})
endif ()

add_library(ffmpeg_node SHARED 
    addon.cpp 
    ffmpeg.cpp
    common.cpp
    parse.cpp 
    update.cpp 
    thumb.cpp
    recording.cpp
)

if(WIN32)
    target_sources(ffmpeg_node PRIVATE
        win_delay_load_hook.c
    )
endif()

set_target_properties(ffmpeg_node PROPERTIES
    PREFIX ""
    SUFFIX ".node"
    OUTPUT_NAME "ffmpeg"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${PP_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PP_OUTPUT_DIR}"
)

target_include_directories(ffmpeg_node PRIVATE
    ${NODE_ADDON_API_DIR}
    ${NODE_API_INCLUDE_DIR}
)

target_compile_definitions(ffmpeg_node PRIVATE
    NAPI_DISABLE_CPP_EXCEPTIONS
)

if (ENABLE_LOG)
    target_compile_definitions(ffmpeg_node PRIVATE ADDON_TRACE)
endif ()

if (FFMPEG_DEBUG)
    target_compile_definitions(ffmpeg_node PRIVATE FFMPEG_DEBUG)
endif ()

if (MINGW)

    target_link_options(ffmpeg_node PRIVATE
        -static-libgcc
        -static-libstdc++
    )

	#set(NODE_LIBRARY "/usr/local/lib/node.lib")
	set(NODE_LIBRARY "/usr/local/lib/libnode-delay.a")

   	target_link_libraries(ffmpeg_node PRIVATE 
		${NODE_LIBRARY}
		delayimp
	)

	target_link_options(ffmpeg_node PRIVATE
   		"-Wl,--delayload=node.exe"
		# "-Wl,--no-undefined"
	)


elseif (LINUX)
    target_link_options(ffmpeg_node PRIVATE
		# "-Wl,-Bsymbolic,--no-undefined"
        "-Wl,-Bsymbolic"
    )
endif ()

target_link_libraries(ffmpeg_node PRIVATE
    ffmpeg
    "$<LINK_GROUP:RESCAN,dav1d,dav1d_bitdepth_8>"
    OpenSSL::SSL OpenSSL::Crypto
    zstd::zstd
    ZLIB::ZLIB
)

if(WIN32)
    target_link_libraries(ffmpeg_node PRIVATE 
		ws2_32 
		bcrypt
        crypt32
	)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET ffmpeg_node POST_BUILD
        COMMAND ${CMAKE_STRIP} $<TARGET_FILE:ffmpeg_node>
    )
endif()


