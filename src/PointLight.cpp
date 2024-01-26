#include <PointLight.hpp>

namespace ENDER {
    glm::vec3 PointLight::position() const {
        return _position;
    }

    glm::vec3 PointLight::ambient() const {
        return _ambient;
    }

    glm::vec3 PointLight::diffuse() const {
        return _diffuse;
    }

    glm::vec3 PointLight::specular() const {
        return _specular;
    }

    float PointLight::constant() const {
        return _constant;
    }

    float PointLight::linear() const {
        return _linear;
    }

    float PointLight::quadratic() const {
        return _quadratic;
    }

    PointLight::PointLight(const glm::vec3 &position, const glm::vec3 &color) : _position(position) {
        type = LightType::PointLight;
        _specular = glm::vec3(1);
        _diffuse = color * glm::vec3(0.5f);
        _ambient = color * glm::vec3(0.2f);
    }
} // ENDER