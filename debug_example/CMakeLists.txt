cmake_minimum_required(VERSION 3.10)
project(debug_example)

# Collect source files
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Create executable
add_executable(debug_example ${SOURCES})

# Link with aurora libraries
# Find Freetype, PNG, and ZLIB explicitly (dependencies of libraries we use)
find_package(Freetype REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)

# Link with aurora libraries
target_link_libraries(debug_example PRIVATE
    aurora_app
    aurora_engine
    fontconfig
    Freetype::Freetype
    PNG::PNG
    ZLIB::ZLIB
)

# Set compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(debug_example PRIVATE -Wall -Wextra -Wpedantic)
endif()
