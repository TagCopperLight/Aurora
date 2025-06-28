cmake_minimum_required(VERSION 3.10)
project(debug_example)

# Collect source files
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Create executable
add_executable(debug_example ${SOURCES})

# Link with aurora libraries
target_link_libraries(debug_example PRIVATE
    aurora_app
    aurora_engine
)

# Set compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(debug_example PRIVATE -Wall -Wextra -Wpedantic)
endif()
