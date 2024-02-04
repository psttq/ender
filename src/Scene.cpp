#include <Scene.hpp>
#include <glm/gtc/matrix_transform.hpp>

ENDER::Scene::Scene() { spdlog::debug("Creating scene."); }

sptr<ENDER::Scene> ENDER::Scene::create() { return sptr<Scene>(new Scene()); }

glm::mat4 ENDER::Scene::calculateView() const {
  auto view = glm::mat4(1.0f);
  if (_camera == nullptr)
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
  else
    view = _camera->getView();
  return view;
}

std::vector<sptr<ENDER::Object>> &ENDER::Scene::getObjects() { return _objects; }

void ENDER::Scene::addObject(sptr<Object> object) { _objects.push_back(object); }

void ENDER::Scene::setCamera(sptr<Camera> camera) { _camera = camera; }

sptr<ENDER::Camera> ENDER::Scene::getCamera() { return _camera; }

void ENDER::Scene::addLight(Light *light) { _lights.push_back(light); }

const std::vector<ENDER::Light *> &ENDER::Scene::getLights() { return _lights; }
