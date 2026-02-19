cmake_minimum_required(VERSION 3.10)
project(cognex)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ZLIB REQUIRED)

# -----------------------------
# CLI Core Sources
# -----------------------------
file(GLOB CLI_SOURCES
    cli/*.cpp
)

file(GLOB CLI_CORE_SOURCES
    cli/src/*.cpp
)

# -----------------------------
# CLI Command Sources
# -----------------------------
file(GLOB CLI_COMMAND_SOURCES
    cli/src/commands/*.cpp
)

# -----------------------------
# Engine Sources
# -----------------------------
file(GLOB ENGINE_SOURCES
    engine/src/*.cpp
)

# -----------------------------
# Bench Sources (optional)
# -----------------------------
file(GLOB BENCH_SOURCES
    bench/*.cpp
)

# -----------------------------
# Executable
# -----------------------------
add_executable(cognex
    ${CLI_SOURCES}
    ${CLI_CORE_SOURCES}
    ${CLI_COMMAND_SOURCES}
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

install(TARGETS cognex DESTINATION bin)
