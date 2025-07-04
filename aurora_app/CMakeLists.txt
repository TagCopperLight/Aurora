cmake_minimum_required(VERSION 3.10)
project(aurora_app)

# Note: CXX standard and packages are set in root CMakeLists.txt

# Collect all source files
file(GLOB_RECURSE SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Add custom command to compile shaders
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert.spv ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag.spv
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../compile_shaders.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag
    COMMENT "Compiling shaders for aurora_app"
    VERBATIM
)

# Add custom target to ensure shader compilation happens
add_custom_target(compile_shaders_app ALL
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert.spv ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag.spv
)

# Create the aurora_app library
add_library(aurora_app STATIC ${SOURCES})

# Make aurora_app depend on shader compilation
add_dependencies(aurora_app compile_shaders_app)

# Set include directories for the library
target_include_directories(aurora_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link aurora_engine library properly using target
target_link_libraries(aurora_app PUBLIC
    aurora_engine  # Proper CMake target dependency
    ${MSDFGEN_CORE_LIB}
    ${MSDFGEN_EXT_LIB}
    ${MSDF_ATLAS_GEN_LIB}
)

# Set compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(aurora_app PRIVATE -Wall -Wextra -Wpedantic)
endif()