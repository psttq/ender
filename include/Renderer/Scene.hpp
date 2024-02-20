#pragma once
#include <Camera.hpp>
#include <Light.hpp>
#include <Object.hpp>
#include <glm/glm.hpp>
#include <vector>

namespace ENDER {
class Scene {
  std::vector<sptr<Object>> _objects;
  sptr<Camera> _camera = nullptr;
  std::vector<Light *> _lights;

  Scene();

public:

  static sptr<Scene> create();

  glm::mat4 calculateView() const;
  void addObject(sptr<Object> object);
  void deleteObject(const sptr<Object>& object);

  void setCamera(sptr<Camera> camera);
  sptr<Camera> getCamera();

  void addLight(Light *light);

  const std::vector<Light *> &getLights();

  std::vector<sptr<Object>> &getObjects();
};
} // namespace ENDER
