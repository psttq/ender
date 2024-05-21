#include "Utilities.hpp"
#include <KinematicSurfaces.hpp>

namespace EGEOM {
KinematicSurface::KinematicSurface(const std::string &name,
                                   const sptr<Spline1> &formingSpline,
                                   const sptr<Spline1> &guideSpline,
                                   const KinematicSurfaceType &type)
    : Surface(name), _formingSpline(formingSpline), _guideSpline(guideSpline),
      _type(type) {
  update();
}
sptr<KinematicSurface>
KinematicSurface::create(const std::string &name,
                         const sptr<Spline1> &formingSpline,
                         const sptr<Spline1> &guideSpline,
                         const KinematicSurface::KinematicSurfaceType &type) {
  return sptr<KinematicSurface>(
      new KinematicSurface(name, formingSpline, guideSpline, type));
}

glm::vec3 KinematicSurface::sweepSplineHelper(float v) {
  return glm::vec3{v + 1000 * v * v, v - 1000 * v, v * v + 1000};
}

glm::vec3 KinematicSurface::pointOnSurface(float u, float v) {
  switch (_type) {
  case KinematicSurfaceType::Shift: {
    auto g = _guideSpline->getSplinePoint(v);
    auto gp0 = g0[0]->getPosition();
    auto c = _formingSpline->getSplinePoint(u);
    glm::vec3 h = {0, 0, 0};
    auto p = g + (c - gp0 - h);
    return p;
  } break;
  case KinematicSurfaceType::Sweep: {
    auto gs = _guideSpline->getSplineDirs(v, 2);
    glm::vec3 d = gs[0]->getPosition() - sweepSplineHelper(v);
    // glm::vec3 d = glm::normalize(
    //     glm::cross(gs[1]->getPosition(), gs[2]->getPosition()));
    auto i1 = glm::normalize(gs[1]->getPosition());
    auto d2 = d - (glm::dot(i1, d)) * i1;
    auto i2 = glm::normalize(d2);
    auto i3 = glm::cross(i1, i2);

    glm::mat3 A{i1, i2, i3};
    auto M = A * Am;
    // // spdlog::info("M[][0] = {} {} {}", M[0][0], M[1][0], M[2][0]);
    // // spdlog::info("M[][1] = {} {} {}", M[0][1], M[1][1], M[2][1]);
    // // spdlog::info("M[][2] = {} {} {}", M[0][2], M[1][2], M[2][2]);
    auto gp0 = g0[0]->getPosition();
    auto g = gs[0]->getPosition();
    auto c = _formingSpline->getSplinePoint(u);
    glm::vec3 h = {0, 0, 0};
    auto p = g + M * (c - gp0 - h);
    return p;
  } break;
  }
};

void KinematicSurface::drawProperties() {
  std::vector<const char *> items = {"Sweep", "Shift"};
  int currentKinematicSurfaceType = static_cast<int>(_type);

  if (ImGui::TreeNode("Kinematic Surface")) {
    if (ImGui::Combo("Kinematic Surface Type", &currentKinematicSurfaceType,
                     &items[0], items.size())) {
      _type = static_cast<KinematicSurfaceType>(currentKinematicSurfaceType);
      update();
    }
    ImGui::TreePop();
  }
}
void KinematicSurface::drawGizmo() {}

void KinematicSurface::update() {
  g0 = _guideSpline->getSplineDirs(0, 3);
  glm::vec3 d = g0[0]->getPosition() - sweepSplineHelper(0);
  auto i10 = glm::normalize(g0[1]->getPosition());
  auto d20 = d - (glm::dot(i10, d)) * i10;
  auto i20 = glm::normalize(d20);
  auto i30 = glm::cross(i10, i20);
  glm::mat3 AAm{i10, i20, i30};
  Am = glm::inverse(AAm);
  auto vao = ENDER::Utils::createParametricSurfaceVAO(
      [&](float u, float v) { return pointOnSurface(u, v); }, 0, 0, 1, 1,
      SURFACE_ROWS, SURFACE_COLS);
  setVertexArray(vao);
}
} // namespace EGEOM
