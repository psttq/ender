#include "Surface.hpp"
#include "Utilities.hpp"
#include "glm/geometric.hpp"
#include <ImGuizmo.h>
#include <PlaneSurface.hpp>

namespace EGEOM {
PlaneSurface::PlaneSurface(const glm::vec3 &a, const glm::vec3 &b,
                           const glm::vec3 &c)
    : EGEOM::Surface("Plane"), _pivot(b) {
  // _basisVec1 = glm::normalize(a - b);
  // _normal = glm::normalize(glm::cross(_basisVec1, c - b));
  // _basisVec2 = glm::normalize(glm::cross(_normal, _basisVec1));
  _basisVec1 = glm::vec3{1, 0, 0};
  _basisVec2 = glm::vec3{0, 0, 1};
  _normal = glm::normalize(glm::cross(_basisVec1, _basisVec2));
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
      1.0, 1.0, 200, 200);
  setVertexArray(vao);
}

std::tuple<glm::vec3, glm::vec3, glm::vec3> PlaneSurface::getBasis() {
  return {_pivot, _basisVec1, _basisVec2};
}

void PlaneSurface::setPivot(const glm::vec3 &pivot) { _pivot = pivot; }

void PlaneSurface::drawGizmo() {
  int n = 5;
  int m = 5;

  auto u_step = 1.0 / (n - 1);
  auto v_step = 1.0 / (m - 1);

  for (auto i = 0; i < n; i++) {
    auto u = i * u_step;
    for (auto j = 0; j < m; j++) {
      auto v = j * v_step;
      auto pc = pointOnSurface(u, v);
      auto pn = normalOnSurface(u, v) + pc;
      ImGuizmo::DrawArrow({pc.x, pc.y, pc.z, 0}, {pn.x, pn.y, pn.z, 0},
                          0xFFFFFF55);
    }
  }
}

void PlaneSurface::drawProperties() {
  ENDER::Object::drawProperties();

  bool shouldUpdate = false;

  if (ImGui::TreeNode("Plane Surface")) {
    shouldUpdate = ImGui::DragFloat("U_min", &u_min, 0.1);
    shouldUpdate = shouldUpdate || ImGui::DragFloat("U_max", &u_max, 0.1, 0,
                                                    glm::pi<float>() * 2);
    shouldUpdate = shouldUpdate || ImGui::DragFloat("V_min", &v_min, 0.1, 0,
                                                    glm::pi<float>() * 2);
    shouldUpdate = shouldUpdate || ImGui::DragFloat("V_max", &v_max, 0.1, 0,
                                                    glm::pi<float>() * 2);
    ImGui::TreePop();
  }
  if (shouldUpdate)
    update();
}

glm::vec3 PlaneSurface::pointOnSurface(float u, float v) {
  u = u_min + (u_max - u_min) * u;
  v = v_min + (v_max - v_min) * v;
  return _basisVec1 * u + _basisVec2 * v + _pivot;
}
glm::vec3 PlaneSurface::normalOnSurface(float u, float v) { return _normal; };
std::tuple<glm::vec3,glm::vec3> PlaneSurface::firstDir(float u, float v){
  return {_basisVec1, _basisVec2};
}
} // namespace EGEOM
