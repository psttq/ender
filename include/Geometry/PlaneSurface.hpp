#pragma once

#include "Surface.hpp"
namespace EGEOM {
class PlaneSurface : public EGEOM::Surface {
  PlaneSurface();

public:
  static sptr<PlaneSurface> create();

  void update() override;

  void drawProperties() override;

  glm::vec3 pointOnSurface(float u, float v) override;
};
} // namespace EGEOM
