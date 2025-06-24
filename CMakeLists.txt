cmake_minimum_required(VERSION 3.10)
project(aurora)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)

file(GLOB SOURCES "src/*.cpp")

add_executable(aurora ${SOURCES})

target_link_libraries(aurora Vulkan::Vulkan glfw dl pthread)