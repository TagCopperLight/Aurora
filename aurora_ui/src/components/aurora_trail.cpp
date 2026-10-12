#include "aurora_ui/components/aurora_trail.hpp"
#include "aurora_ui/utils/aurora_theme_settings.hpp"

#include <algorithm>

namespace aurora {
    namespace {
        // Newest segment strength (blend towards the trail color) and the thickness of the oldest segment relative to the newest
        constexpr float MAX_STRENGTH = 0.6f;
        constexpr float MIN_SIZE = 0.4f;
    }

    AuroraTrail::AuroraTrail(AuroraComponentInfo& componentInfo, size_t length, float thickness, glm::vec4 color)
        : AuroraComponentInterface{componentInfo}, points(length), thickness{thickness}, trailColor{color} {
        // One segment joins each pair of consecutive points
        for (size_t i = 1; i < length; ++i) {
            auto segment = std::make_shared<AuroraSegment>(componentInfo, color);
            segment->setHidden(true);
            segments.push_back(segment);
            addChild(segment);
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

        // Segment `age` joins the point of that age to the next older one
        for (size_t age = 0; age < segments.size(); ++age) {
            auto& segment = segments[age];
            if (age + 1 >= size) {
                segment->setHidden(true);
                continue;
            }

            size_t newer = (head + length - 1 - age) % length;
            size_t older = (head + length - 2 - age) % length;
            float fade = 1.f - static_cast<float>(age) / static_cast<float>(length);
            float segmentThickness = thickness * (MIN_SIZE + (1.f - MIN_SIZE) * fade);

            segment->color = glm::mix(background, trailColor, MAX_STRENGTH * fade);
            segment->color.a = 1.f;
            segment->setEndpoints(points[newer], points[older], segmentThickness);
            segment->setHidden(false);
        }
    }
}
