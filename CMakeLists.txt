cmake_minimum_required(VERSION 3.10)
project(cognex)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ZLIB REQUIRED)

# -----------------------------
# Collect Source Files
# -----------------------------

file(GLOB_RECURSE ENGINE_SOURCES
    engine/src/*.cpp
)

file(GLOB_RECURSE CLI_SOURCES
    cli/*.cpp
)

file(GLOB_RECURSE BENCH_SOURCES
    bench/*.cpp
)

set(CLI_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/cli/main.cpp")
list(REMOVE_ITEM CLI_SOURCES "${CLI_MAIN}")

# -----------------------------
# Executable
# -----------------------------

add_executable(cognex
    ${ENGINE_SOURCES}
    ${CLI_SOURCES}
    ${CLI_MAIN}
)

add_executable(cognex_bench
    ${ENGINE_SOURCES}
    ${BENCH_SOURCES}
)

# -----------------------------
# Include Directories
# -----------------------------

target_include_directories(cognex
    PRIVATE
        engine/include
        cli/include
)

target_include_directories(cognex_bench
    PRIVATE
        engine/include
)

# -----------------------------
# Compile Definitions
# -----------------------------

target_compile_definitions(cognex
    PRIVATE
        COGNEX_DEBUG=0
)

target_compile_definitions(cognex_bench
    PRIVATE
        COGNEX_DEBUG=0
)

# -----------------------------
# Link Libraries
# -----------------------------

target_link_libraries(cognex
    PRIVATE
        ZLIB::ZLIB
)

target_link_libraries(cognex_bench
    PRIVATE
        ZLIB::ZLIB
)

# -----------------------------
# Install
# -----------------------------

install(TARGETS cognex cognex_bench DESTINATION bin)