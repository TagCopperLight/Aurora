cmake_minimum_required(VERSION 3.10)
project(Aurora VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add profiling flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")

# Find required packages once at the root level
find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(glm REQUIRED)

# Find MSDF libraries manually
find_library(MSDFGEN_CORE_LIB msdfgen-core)
find_library(MSDFGEN_EXT_LIB msdfgen-ext)
find_library(MSDF_ATLAS_GEN_LIB msdf-atlas-gen)

if(NOT MSDFGEN_CORE_LIB)
    message(FATAL_ERROR "msdfgen-core library not found")
endif()

if(NOT MSDFGEN_EXT_LIB)
    message(FATAL_ERROR "msdfgen-ext library not found")
endif()

if(NOT MSDF_ATLAS_GEN_LIB)
    message(FATAL_ERROR "msdf-atlas-gen library not found")
endif()

# Add subdirectories in dependency order
add_subdirectory(aurora_engine)
add_subdirectory(aurora_app)
add_subdirectory(debug_example)

# Optional: Create an install target
install(TARGETS aurora_engine aurora_app
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
