#pragma once

#include "Object.hpp"
namespace EGEOM {

const uint SURFACE_ROWS = 100;
const uint SURFACE_COLS = 100;

class Surface : public ENDER::Object {
public:
  Surface(const std::string &name);

  virtual glm::vec3 pointOnSurface(float u, float v) = 0;
};

} // namespace EGEOM
