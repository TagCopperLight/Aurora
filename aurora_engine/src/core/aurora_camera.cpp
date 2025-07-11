#include "aurora_engine/core/aurora_camera.hpp"

namespace aurora {
    void AuroraCamera::setOrthographicProjection(float left, float right, float bottom, float top, float near, float far) {
        projectionMatrix = glm::mat4(1.0f);
        projectionMatrix[0][0] = 2.0f / (right - left);
        projectionMatrix[1][1] = 2.0f / (bottom - top);
        projectionMatrix[2][2] = 1.0f / (far - near);
        projectionMatrix[3][0] = -(right + left) / (right - left);
        projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
        projectionMatrix[3][2] = -near / (far - near);
    }
}