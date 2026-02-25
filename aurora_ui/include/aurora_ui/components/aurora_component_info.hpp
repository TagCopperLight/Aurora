#pragma once

#include "aurora_engine/core/aurora_device.hpp"

namespace aurora {
    class AuroraRenderSystemManager;
    
    struct AuroraComponentInfo {
        AuroraDevice &auroraDevice;
        AuroraRenderSystemManager &renderSystemManager;
    };
}