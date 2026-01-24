cmake_minimum_required(VERSION 3.10)
project(cognex_v2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find zlib for CRC32
find_package(ZLIB REQUIRED)

add_executable(cognex
    src/main.cpp
    src/cognex.cpp
    src/wal.cpp
    src/snapshot.cpp
)

target_include_directories(cognex
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(cognex
    PRIVATE
        ZLIB::ZLIB
)
