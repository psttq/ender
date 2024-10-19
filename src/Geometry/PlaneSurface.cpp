#include "Surface.hpp"
#include "Utilities.hpp"
#include <PlaneSurface.hpp>

namespace EGEOM {
PlaneSurface::PlaneSurface() : EGEOM::Surface("Plane") { update(); }

sptr<PlaneSurface> PlaneSurface::create() {
  return sptr<PlaneSurface>(new PlaneSurface());
}

void PlaneSurface::update() {
  auto vao = ENDER::Utils::createParametricSurfaceVAO(
      [&](float u, float v) { return pointOnSurface(u, v); }, u_min, v_min,
      u_max, v_max, 200, 200);
  setVertexArray(vao);
}

void PlaneSurface::drawProperties() {}

glm::vec3 PlaneSurface::pointOnSurface(float u, float v) { return glm::vec3{u, 0, v}; }
} // namespace EGEOM
