cmake_minimum_required(VERSION 3.10)
project(aurora_debug)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(aurora_debug STATIC ${SOURCES})

target_include_directories(aurora_debug PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(aurora_debug PUBLIC
    spdlog::spdlog
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(aurora_debug PRIVATE -Wall -Wextra -Wpedantic)
endif()