cmake_minimum_required(VERSION 3.10)
project(aurora_bench)

# Collect source files
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Create executable
add_executable(aurora_bench ${SOURCES})

# Link with aurora libraries
# Find Freetype, PNG, and ZLIB explicitly (dependencies of libraries we use)
find_package(Freetype REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)

# Link with aurora libraries
target_link_libraries(aurora_bench PRIVATE
    aurora_ui
    aurora_engine
    Freetype::Freetype
    PNG::PNG
    ZLIB::ZLIB
)

# Set compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(aurora_bench PRIVATE -Wall -Wextra -Wpedantic)
endif()
