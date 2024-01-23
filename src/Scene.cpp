#include <Scene.hpp>
#include <glm/gtc/matrix_transform.hpp>

ENDER::Scene::Scene() {
    spdlog::debug("Creating scene.");
}

glm::mat4 ENDER::Scene::calculateView() {
    auto view = glm::mat4(
        1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    return view;
}

std::vector<ENDER::Object *> &ENDER::Scene::getObjects() {
    return _objects;
}

void ENDER::Scene::addObject(Object *object) {
    _objects.push_back(object);
}

