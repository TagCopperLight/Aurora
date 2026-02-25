#include "aurora_ui/components/aurora_component_interface.hpp"
#include "aurora_ui/graphics/aurora_render_system_manager.hpp"

namespace aurora {
    void AuroraComponentInterface::addToRenderSystem() {
        componentInfo.renderSystemManager.addComponentToQueue(shared_from_this());
        rendering = true;

        for (auto& child : children) {
            child->addToRenderSystem();
        }
    }
}
