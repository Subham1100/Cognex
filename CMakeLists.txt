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

file(GLOB BENCH_SOURCES
    bench/*.cpp
)

# -----------------------------
# Executable
# -----------------------------

add_executable(cognex
    ${ENGINE_SOURCES}
    ${CLI_SOURCES}
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

# -----------------------------
# Compile Definitions
# -----------------------------

target_compile_definitions(cognex
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

# -----------------------------
# Install
# -----------------------------

install(TARGETS cognex DESTINATION bin)