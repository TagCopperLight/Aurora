cmake_minimum_required(VERSION 3.10)
project(aurora_ui)

# Note: CXX standard and packages are set in root CMakeLists.txt

# Find glslc compiler for shader compilation
find_program(GLSLC glslc REQUIRED)

# Collect all source files
file(GLOB_RECURSE SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Collect all shader files
file(GLOB VERTEX_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert")
file(GLOB FRAGMENT_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag")

# Create build/shaders directory
set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

# Function to compile shaders
function(compile_shader SHADER_FILE OUTPUT_FILE)
    add_custom_command(
        OUTPUT ${OUTPUT_FILE}
        COMMAND ${GLSLC} ${SHADER_FILE} -o ${OUTPUT_FILE}
        DEPENDS ${SHADER_FILE}
        COMMENT "Compiling shader: ${SHADER_FILE}"
    )
endfunction()

# Compile vertex shaders
set(COMPILED_VERTEX_SHADERS)
foreach(SHADER_FILE ${VERTEX_SHADERS})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    set(OUTPUT_FILE "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv")
    compile_shader(${SHADER_FILE} ${OUTPUT_FILE})
    list(APPEND COMPILED_VERTEX_SHADERS ${OUTPUT_FILE})
endforeach()

# Compile fragment shaders
set(COMPILED_FRAGMENT_SHADERS)
foreach(SHADER_FILE ${FRAGMENT_SHADERS})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    set(OUTPUT_FILE "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv")
    compile_shader(${SHADER_FILE} ${OUTPUT_FILE})
    list(APPEND COMPILED_FRAGMENT_SHADERS ${OUTPUT_FILE})
endforeach()

# Create a custom target for shader compilation
add_custom_target(compile_shaders_app ALL
    DEPENDS ${COMPILED_VERTEX_SHADERS} ${COMPILED_FRAGMENT_SHADERS}
    COMMENT "Compiling all aurora_ui shaders"
)

# Create the aurora_ui library
add_library(aurora_ui STATIC ${SOURCES})

# Make sure shaders are compiled before the library
add_dependencies(aurora_ui compile_shaders_app)

# Enable profiling globally for the app
target_compile_definitions(aurora_ui PUBLIC AURORA_PROFILING_ENABLED)

# Set include directories for the library
target_include_directories(aurora_ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Copy assets folder to build directory
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            ${CMAKE_BINARY_DIR}/assets
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/assets
    COMMENT "Copying assets to build directory"
)

add_custom_target(copy_assets ALL
    DEPENDS ${CMAKE_BINARY_DIR}/assets
    COMMENT "Ensuring assets are copied to build directory"
)

# Make sure assets are copied before the library
add_dependencies(aurora_ui copy_assets)

# Link aurora_engine library properly using target
target_link_libraries(aurora_ui PUBLIC
    aurora_engine  # Proper CMake target dependency
    ${MSDF_ATLAS_GEN_LIB}
    ${MSDFGEN_EXT_LIB}
    ${MSDFGEN_CORE_LIB}
)

# Set compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(aurora_ui PRIVATE -Wall -Wextra -Wpedantic)
endif()