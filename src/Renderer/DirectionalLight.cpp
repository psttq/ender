#include <DirectionalLight.hpp>

void ENDER::DirectionalLight::setDirection(const glm::vec3 &direction) {
    _direction = direction;
}

glm::vec3 ENDER::DirectionalLight::ambient() const {
    return _ambient;
}

glm::vec3 ENDER::DirectionalLight::diffuse() const {
    return _diffuse;
}

glm::vec3 ENDER::DirectionalLight::specular() const {
    return _specular;
}

glm::vec3 ENDER::DirectionalLight::direction() const {
    return _direction;
}

ENDER::DirectionalLight::DirectionalLight(const glm::vec3 &direction, const glm::vec3 &color) : _direction(direction){
    type = LightType::DirectionalLight;
    _specular = glm::vec3(0.5);
    _diffuse = color * glm::vec3(0.7f);
    _ambient = color * glm::vec3(0.4f);
}
