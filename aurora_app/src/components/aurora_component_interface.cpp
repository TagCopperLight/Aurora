#include "aurora_app/components/aurora_component_interface.hpp"
#include "aurora_app/graphics/aurora_render_system_manager.hpp"

namespace aurora {
    void AuroraComponentInterface::addToRenderSystem() {
        componentInfo.renderSystemManager.addComponentToQueue(shared_from_this());
        rendering = true;

        for (auto& child : children) {
            child->addToRenderSystem();
        }
    }
}
