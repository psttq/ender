#pragma once

#include "Surface.hpp"
namespace EGEOM {
class PlaneSurface : public EGEOM::Surface {
  PlaneSurface(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c);

  glm::vec3 _basisVec1;
  glm::vec3 _basisVec2;
  glm::vec3 _pivot;

  glm::vec3 _normal;

public:
  static sptr<PlaneSurface> create(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c);

  void update() override;

  void drawProperties() override;

  glm::vec3 pointOnSurface(float u, float v) override;
};
} // namespace EGEOM
