#pragma once
#include <glm/glm.hpp>

namespace ENDER {
    class Camera {
    public:
        virtual ~Camera() = default;

        virtual glm::mat4 getView() const = 0;
    };
}
