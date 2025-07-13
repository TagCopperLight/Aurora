#include "aurora_app/components/aurora_card.hpp"
#include "aurora_app/components/aurora_rounded_borders.hpp"
#include "aurora_app/components/aurora_rounded_rect.hpp"
#include "aurora_app/components/aurora_rounded_shadows.hpp"
#include "aurora_app/components/aurora_text.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/constants.hpp>

#include <memory>

namespace aurora {
    AuroraCard::AuroraCard(AuroraComponentInfo &componentInfo, glm::vec2 size, glm::vec4 borderColor)
        : AuroraComponentInterface{componentInfo}, size{size}, borderColor{borderColor} {
        initialize();
    }

    void AuroraCard::initialize() {
        float radius = 50.0f;

        auto shadowBordersComponent = std::make_shared<AuroraRoundedShadows>(componentInfo, size, radius, 15.0f);

        auto bordersComponent = std::make_shared<AuroraRoundedBorders>(componentInfo, size, radius, 6.0f);
        bordersComponent->color = borderColor;

        auto roundedRectComponent = std::make_shared<AuroraRoundedRectangle>(componentInfo, size, radius);
        roundedRectComponent->color = {0.196f, 0.196f, 0.196f, 1.0f};

        addChild(shadowBordersComponent);
        addChild(roundedRectComponent);
        addChild(bordersComponent);
    }
}