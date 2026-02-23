#include "aurora_app/utils/aurora_theme_settings.hpp"

#include <cstdio>
#include <cstring>
#include <cmath>

namespace aurora {

    static float toLinear(float c) {
        return std::pow(c, 2.2f);
    }

    static float toSRGB(float c) {
        return c <= 0.0031308f
            ? c * 12.92f
            : 1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f;
    }

    glm::vec4 AuroraThemeSettings::fromHex(const char* hex) {
        unsigned int r = 0, g = 0, b = 0, a = 255;

        const std::size_t len = std::strlen(hex);
        if (len == 7) {
            std::sscanf(hex + 1, "%02x%02x%02x", &r, &g, &b);
        } else if (len == 9) {
            std::sscanf(hex + 1, "%02x%02x%02x%02x", &r, &g, &b, &a);
        }

        // Pre-compensate for Hyprland's sRGB→Gamma2.2 color management transform.
        // Hyprland applies: sRGB_EOTF(x) → linear → Gamma22_OETF(x) to every pixel.
        // Storing toSRGB(toLinear(c)) = sRGB_OETF(Gamma22_EOTF(c)) is the exact inverse,
        // so the net result through Hyprland's CM pipeline is the original hex color.
        // Alpha is not color-managed.
        auto cm = [](float c) { return toSRGB(toLinear(c)); };

        return glm::vec4(
            cm(r / 255.0f),
            cm(g / 255.0f),
            cm(b / 255.0f),
            a / 255.0f
        );
    }

    AuroraThemeSettings::Colors AuroraThemeSettings::init() {
        Colors c{};

        c.BACKGROUND = fromHex(BACKGROUND);
        c.PURPLE     = fromHex(PURPLE);
        c.BLUE       = fromHex(BLUE);
        c.ORANGE     = fromHex(ORANGE);
        c.GRAY       = fromHex(GRAY);
        c.DELIMITER  = fromHex(DELIMITER);

        c.TEXT_PRIMARY   = fromHex(TEXT_PRIMARY);
        c.TEXT_SECONDARY = fromHex(TEXT_SECONDARY);
        c.TEXT_DISABLED  = fromHex(TEXT_DISABLED);

        c.SHADOW_LIGHT       = fromHex(SHADOW_LIGHT);
        c.SHADOW_MEDIUM      = fromHex(SHADOW_MEDIUM);
        c.SHADOW_HEAVY       = fromHex(SHADOW_HEAVY);
        c.SHADOW_TRANSPARENT = fromHex(SHADOW_TRANSPARENT);

        c.SUCCESS = fromHex(SUCCESS);
        c.ERROR   = fromHex(ERROR);

        c.TRANSPARENT = fromHex(TRANSPARENT);
        c.WHITE       = fromHex(WHITE);
        c.BLACK       = fromHex(BLACK);
        
        return c;
    }

    const AuroraThemeSettings::Colors& AuroraThemeSettings::get() {
        static const Colors instance = init();
        return instance;
    }

}