#include "Surface.hpp"
#include "Utilities.hpp"
#include "glm/geometric.hpp"
#include <PlaneSurface.hpp>

namespace EGEOM {
PlaneSurface::PlaneSurface(const glm::vec3 &a, const glm::vec3 &b,
                           const glm::vec3 &c)
    : EGEOM::Surface("Plane"), _pivot(b) {
  _basisVec1 = glm::normalize(a - b);
  _normal = glm::normalize(glm::cross(_basisVec1, c - b));
  _basisVec2 = glm::normalize(glm::cross(_normal, _basisVec1));

  update();
}

// Point b is pivot point
sptr<PlaneSurface> PlaneSurface::create(const glm::vec3 &a, const glm::vec3 &b,
                                        const glm::vec3 &c) {
  return sptr<PlaneSurface>(new PlaneSurface(a, b, c));
}

void PlaneSurface::update() {
  auto vao = ENDER::Utils::createParametricSurfaceVAO(
      [&](float u, float v) { return pointOnSurface(u, v); }, u_min, v_min,
      u_max, v_max, 200, 200);
  setVertexArray(vao);
}

std::tuple<glm::vec3, glm::vec3, glm::vec3> PlaneSurface::getBasis() {
  return {_pivot, _basisVec1, _basisVec2};
}

void PlaneSurface::setPivot(const glm::vec3 &pivot) { _pivot = pivot; }

void PlaneSurface::drawProperties() {}

glm::vec3 PlaneSurface::pointOnSurface(float u, float v) {
  return _basisVec1 * u + _basisVec2 * v + _pivot;
}
glm::vec3 PlaneSurface::normalOnSurface(float u, float v) { return _normal; };
} // namespace EGEOM
