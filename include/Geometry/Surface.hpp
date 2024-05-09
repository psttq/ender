#pragma once

#include "Object.hpp"
namespace EGEOM {

class Surface : public ENDER::Object {
public:
  Surface(const std::string &name);

  virtual glm::vec3 pointOnSurface(float u, float v) = 0;

};

} //EGEOM
