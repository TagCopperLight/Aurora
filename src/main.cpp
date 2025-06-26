// #include "aurora_app.hpp"
#include "aurora_app.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <spdlog/spdlog.h>

int main(){
    spdlog::set_level(spdlog::level::info);
    
    spdlog::info("Starting Aurora Vulkan Application");
    aurora::AuroraApp app{};

    try {
        app.run();
    } catch (const std::exception &e) {
        spdlog::error("{}", e.what());
        return EXIT_FAILURE;
    }

    spdlog::info("Aurora Vulkan Application exiting successfully");
    return EXIT_SUCCESS;
}