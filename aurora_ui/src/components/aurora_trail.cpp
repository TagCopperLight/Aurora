#include "aurora_ui/components/aurora_trail.hpp"
#include "aurora_ui/utils/aurora_theme_settings.hpp"

#include <algorithm>

namespace aurora {
    namespace {
        // Newest dot strength (blend towards the trail color) and the size of the oldest dot relative to the newest
        constexpr float MAX_STRENGTH = 0.6f;
        constexpr float MIN_SIZE = 0.4f;
    }

    AuroraTrail::AuroraTrail(AuroraComponentInfo& componentInfo, size_t length, float radius, glm::vec4 color)
        : AuroraComponentInterface{componentInfo}, points(length), radius{radius}, trailColor{color} {
        for (size_t i = 0; i < length; ++i) {
            auto dot = std::make_shared<AuroraCircle>(componentInfo, 1.f, color);
            dot->setHidden(true);
            dots.push_back(dot);
            addChild(dot);
        }
    }

    void AuroraTrail::push(glm::vec2 position) {
        if (points.empty()) return;
        points[head] = position;
        head = (head + 1) % points.size();
        size = std::min(size + 1, points.size());
        refresh();
    }

    void AuroraTrail::clear() {
        size = 0;
        refresh();
    }

    void AuroraTrail::setColor(glm::vec4 color) {
        if (trailColor == color) return;
        trailColor = color;
        refresh();
    }

    void AuroraTrail::refresh() {
        const auto& background = AuroraThemeSettings::get().BACKGROUND;
        size_t length = points.size();

        for (size_t age = 0; age < length; ++age) {
            auto& dot = dots[age];
            if (age >= size) {
                dot->setHidden(true);
                continue;
            }

            size_t index = (head + length - 1 - age) % length;
            float fade = 1.f - static_cast<float>(age) / static_cast<float>(length);
            float dotRadius = radius * (MIN_SIZE + (1.f - MIN_SIZE) * fade);

            dot->color = glm::mix(background, trailColor, MAX_STRENGTH * fade);
            dot->color.a = 1.f;
            dot->setPosition(points[index]);
            dot->setScale(dotRadius, dotRadius);
            dot->setHidden(false);
        }
    }
}
