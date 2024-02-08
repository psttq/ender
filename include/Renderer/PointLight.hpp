#pragma once

#include <Light.hpp>
#include <../../3rd/glm/glm/glm.hpp>

namespace ENDER {
class PointLight : public Light{
    glm::vec3 _position{};
    glm::vec3 _ambient{};
    glm::vec3 _diffuse{};
    glm::vec3 _specular{};

    float _constant = 1.0f;
    float _linear = 0.09f;
    float _quadratic = 0.032f;

public:
    glm::vec3 position() const;

    glm::vec3 ambient() const;

    glm::vec3 diffuse() const;

    glm::vec3 specular() const;

    float constant() const;

    float linear() const;

    float quadratic() const;

    PointLight(const glm::vec3 &position, const glm::vec3 &color);



};
} // ENDER
