cmake_minimum_required(VERSION 3.10)
project(cognex)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find zlib (for WAL CRC)
find_package(ZLIB REQUIRED)

file(GLOB CLI_SOURCES cli/*.cpp)
file(GLOB ENGINE_SOURCES engine/src/*.cpp)
file(GLOB BENCH_SOURCES bench/*.cpp)

add_executable(cognex
    ${CLI_SOURCES}
    ${ENGINE_SOURCES}
    ${BENCH_SOURCES}
)

install(TARGETS cognex DESTINATION bin)


target_include_directories(cognex
    PRIVATE
        engine/include
        cli
)

target_compile_definitions(cognex
    PRIVATE
        COGNEX_DEBUG=0
)


target_link_libraries(cognex
    PRIVATE
        ZLIB::ZLIB
)
