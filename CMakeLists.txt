cmake_minimum_required(VERSION 3.16)
project(voice_chat VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 添加编译选项
if(MSVC)
    add_compile_options(/W4 /MP)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 查找依赖包
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(Protobuf REQUIRED)
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# 查找 PortAudio 库和头文件
find_library(PORTAUDIO_LIBRARIES
    NAMES portaudio
    PATHS /usr/lib/x86_64-linux-gnu/
    REQUIRED
)

find_path(PORTAUDIO_INCLUDE_DIRS
    NAMES portaudio.h
    PATHS /usr/include
    REQUIRED
)

# Opus库配置
if(NOT OPUS_ROOT_DIR)
    set(OPUS_ROOT_DIR "${CMAKE_SOURCE_DIR}/lib/opus" CACHE PATH "Opus library root directory")
endif()

find_library(OPUS_LIBRARY
    NAMES opus
    PATHS ${OPUS_ROOT_DIR}/lib /usr/lib/x86_64-linux-gnu/
    PATH_SUFFIXES lib lib64
    REQUIRED
)

find_path(OPUS_INCLUDE_DIR
    NAMES opus/opus.h
    PATHS ${OPUS_ROOT_DIR}/include /usr/include
    PATH_SUFFIXES include
    REQUIRED
)

# 设置包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS}
    ${PORTAUDIO_INCLUDE_DIRS}
    ${OPUS_INCLUDE_DIR}
)

# 添加子目录
add_subdirectory(proto)
add_subdirectory(src)
add_subdirectory(gui)

# 打印配置信息
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Boost version: ${Boost_VERSION}")
message(STATUS "Protobuf version: ${Protobuf_VERSION}")
message(STATUS "Opus library: ${OPUS_LIBRARY}")
message(STATUS "PortAudio found: ${PORTAUDIO_LIBRARIES}") 