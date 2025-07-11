#include "aurora_app/components/aurora_card.hpp"
#include "aurora_app/components/aurora_rounded_borders.hpp"
#include "aurora_app/components/aurora_rounded_rect.hpp"
#include "aurora_app/components/aurora_rounded_shadows.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/constants.hpp>

#include <memory>

namespace aurora {
    AuroraCard::AuroraCard(AuroraDevice &device, glm::vec2 size, glm::vec4 borderColor, std::string title)
        : AuroraComponentInterface{device}, size{size}, borderColor{borderColor}, title{title} {
        initialize();
    }

    void AuroraCard::initialize() {
        float radius = 50.0f;

        auto bordersComponent = std::make_unique<AuroraRoundedBorders>(auroraDevice, size, radius, 6.0f);
        bordersComponent->color = borderColor;
        float borderZ = bordersComponent->transform.translation.z;

        auto roundedRectComponent = std::make_unique<AuroraRoundedRectangle>(auroraDevice, size, radius);
        roundedRectComponent->color = {0.196f, 0.196f, 0.196f, 1.0f};
        roundedRectComponent->transform.translation.z = borderZ + 0.01f;

        auto shadowBordersComponent = std::make_unique<AuroraRoundedShadows>(auroraDevice, size, radius, 15.0f);

        addChild(std::move(bordersComponent));
        addChild(std::move(roundedRectComponent));
        addChild(std::move(shadowBordersComponent));
    }
}