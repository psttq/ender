#pragma once

#include "Object.hpp"
namespace EGEOM {

const uint SURFACE_ROWS = 100;
const uint SURFACE_COLS = 100;

class Surface : public ENDER::Object {
protected:
  float u_min;
  float u_max;
  float v_min;
  float v_max;

public:
  Surface(const std::string &name);

  virtual void update() = 0;

  std::tuple<float, float, float, float> getUVMinMax();

  virtual glm::vec3 pointOnSurface(float u, float v) = 0;
  virtual glm::vec3 normalOnSurface(float u, float v) { return {}; }
};

} // namespace EGEOM
