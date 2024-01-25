#pragma once
#include <glm/glm.hpp>

namespace ENDER {
    class Camera {
    public:
        virtual ~Camera() = default;

        virtual glm::mat4 getView() const = 0;
        virtual glm::vec3 getPosition() const = 0;
        virtual glm::vec3 getFront() const = 0;
        virtual bool getSpotlightToggled() const = 0;

    };
}
