#pragma once
#include <glm/glm.hpp>

namespace ENDER{
struct Material{
  glm::vec3 ambient {1.0f, 0.5f, 0.31f};
  glm::vec3 diffuse{1.0f, 0.5f, 0.31f};
  glm::vec3 specular{0.5f, 0.5f, 0.5f};
  float shininess = 64.f; 

  void drawImguiEdit();
};
}
