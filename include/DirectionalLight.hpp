#pragma once
#include "Light.hpp"
#include <glm/glm.hpp>

namespace ENDER {
    class DirectionalLight : public Light{
        glm::vec3 _ambient{};
        glm::vec3 _diffuse{};
        glm::vec3 _specular{};
        glm::vec3 _direction{};

    public:
        void setDirection(const glm::vec3 &direction);

        glm::vec3 ambient() const;

        glm::vec3 diffuse() const;

        glm::vec3 specular() const;

        glm::vec3 direction() const;

        DirectionalLight(const glm::vec3 &direction, const glm::vec3 &color);

    };
};