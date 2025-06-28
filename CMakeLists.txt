cmake_minimum_required(VERSION 3.10)
project(Aurora VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages once at the root level
find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(spdlog REQUIRED)

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
