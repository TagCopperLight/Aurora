cmake_minimum_required(VERSION 3.10)
project(aurora)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)

file(GLOB SOURCES "src/*.cpp")

# Add custom command to compile shaders
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert.spv ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag.spv
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/compile.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag
    COMMENT "Compiling shaders"
    VERBATIM
)

# Add custom target to ensure shader compilation happens
add_custom_target(compile_shaders ALL
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert.spv ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag.spv
)

add_executable(aurora ${SOURCES})

# Make aurora depend on shader compilation
add_dependencies(aurora compile_shaders)

target_link_libraries(aurora Vulkan::Vulkan glfw dl pthread)