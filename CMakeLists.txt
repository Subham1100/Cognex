cmake_minimum_required(VERSION 3.10)
project(cognex)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find zlib (for WAL CRC)
find_package(ZLIB REQUIRED)

add_executable(cognex
    cli/main.cpp
    cli/repl.cpp

    engine/src/cognex.cpp
    engine/src/wal.cpp
    engine/src/snapshot.cpp
)

install(TARGETS cognex DESTINATION bin)


target_include_directories(cognex
    PRIVATE
        engine/include
        cli
)

target_link_libraries(cognex
    PRIVATE
        ZLIB::ZLIB
)
