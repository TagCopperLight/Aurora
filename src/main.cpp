#include "aurora_app.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main(){
    aurora::AuroraApp app{};

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}