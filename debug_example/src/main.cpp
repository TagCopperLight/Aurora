// #include "aurora_app.hpp"
#include "aurora_app/aurora_app.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <spdlog/spdlog.h>
#include <fontconfig/fontconfig.h>

int main(){
    spdlog::set_level(spdlog::level::info);

    FcInit();

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