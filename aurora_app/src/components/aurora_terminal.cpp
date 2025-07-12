#include "aurora_app/components/aurora_terminal.hpp"
#include "aurora_app/components/aurora_card.hpp"
#include "aurora_app/components/aurora_text.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/constants.hpp>

#include <memory>

namespace aurora {
    AuroraTerminal::AuroraTerminal(AuroraComponentInfo &componentInfo, glm::vec2 size)
        : AuroraComponentInterface{componentInfo}, size{size} {
        initialize();
    }

    void AuroraTerminal::initialize() {
        auto cardComponent = std::make_unique<AuroraCard>(componentInfo, size, glm::vec4(0.784f, 0.38f, 0.286f, 1.0f));

        auto textComponent = std::make_unique<AuroraText>(componentInfo, "> Hello World !", 15.0f);
        textComponent->setPosition(50 * 0.8f, 50 * 0.5f);

        addChild(std::move(cardComponent));
        addChild(std::move(textComponent));
    }
}