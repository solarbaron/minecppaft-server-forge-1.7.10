cmake_minimum_required(VERSION 3.20)
project(minecppaft-server
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "Minecraft 1.7.10 server rewrite in C++"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Main executable
add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Future: threading, networking (asio), zlib, etc.
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
