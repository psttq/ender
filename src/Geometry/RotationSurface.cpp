#include "Utilities.hpp"
#include "imgui.h"
#include <RotationSurface.hpp>

namespace EGEOM {
RotationSurface::RotationSurface(const std::string &name,
                                 sptr<Spline1> baseSpline, float rotationAngle,
                                 float rotationRadius)
    : Surface(name), _baseSpline(baseSpline), _rotationRadius(rotationRadius),
      _rotationAngle(rotationAngle) {
  update();
}

sptr<RotationSurface> RotationSurface::create(const std::string &name, sptr<Spline1> baseSpline,
                  float rotationAngle, float rotationRadius){
    return sptr<RotationSurface>(new RotationSurface(name, baseSpline,rotationAngle, rotationRadius));
}

void RotationSurface::update() { // FIXME: set vbo data ideally but i am too
                                 // lazy now
  auto vao = ENDER::Utils::createParametricSurfaceVAO(
      [&](float u, float v) { return pointOnSurface(u, v); }, 0, 0, 1,
      _rotationAngle, SURFACE_ROWS, SURFACE_COLS);

  setVertexArray(vao);
}

glm::vec3 RotationSurface::pointOnSurface(float u, float v) {
  auto splinePoint =
      _baseSpline->getSplinePoint(u) + glm::vec3{-_rotationRadius, 0, 0};
  return glm::vec3{_rotationRadius, 0, 0} +
         glm::vec3{splinePoint.x * glm::cos(v), splinePoint.x * glm::sin(v),
                   splinePoint.z};
}

void RotationSurface::drawProperties() {
  bool shouldUpdate = false;

  if (ImGui::TreeNode("Rotation Surface")) {
    shouldUpdate = ImGui::DragFloat("Rotate radius", &_rotationRadius, 0.1);
    shouldUpdate = ImGui::DragFloat("Rotate angle", &_rotationAngle, 0.1, 0,
                                    glm::pi<float>() * 2);
    ImGui::TreePop();
  }
  if (shouldUpdate)
      update();
}
} // namespace EGEOM
