cmake_minimum_required(VERSION 3.10)
project(cognex_v0)

set(CMAKE_CXX_STANDARD 17)

add_executable(cognex
    src/main.cpp
    src/cognex.cpp
    src/wal.cpp
)
target_include_directories(cognex
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)
