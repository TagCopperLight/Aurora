cmake_minimum_required(VERSION 3.10)
project(aurora_engine)

# Note: CXX standard and packages are set in root CMakeLists.txt

# Collect all source files
file(GLOB_RECURSE SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Create the aurora_engine library
add_library(aurora_engine STATIC ${SOURCES})

# Set include directories for the library
target_include_directories(aurora_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link required libraries
target_link_libraries(aurora_engine PUBLIC
    Vulkan::Vulkan
    glfw
    spdlog::spdlog
    dl
    pthread
)

# Set compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(aurora_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_compile_definitions(aurora_engine PUBLIC AURORA_PROFILING_ENABLED)