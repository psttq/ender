#include "Object.hpp"
#include "Utilities.hpp"
#include "imgui.h"
#include <ImGuizmo.h>
#include <RotationSurface.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace EGEOM {
RotationSurface::RotationSurface(const std::string &name,
                                 sptr<Spline1> baseSpline, float rotationAngle,
                                 float rotationRadius)
    : Surface(name), _baseSpline(baseSpline), _rotationRadius(rotationRadius),
      _rotationAngle(rotationAngle) {
  update();
}

sptr<RotationSurface> RotationSurface::create(const std::string &name,
                                              sptr<Spline1> baseSpline,
                                              float rotationAngle,
                                              float rotationRadius) {
  return sptr<RotationSurface>(
      new RotationSurface(name, baseSpline, rotationAngle, rotationRadius));
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

  ENDER::Object::drawProperties();

  bool shouldUpdate = false;

  if (ImGui::TreeNode("Rotation Surface")) {
    shouldUpdate = ImGui::DragFloat("Rotate radius", &_rotationRadius, 0.1);
    shouldUpdate =
        shouldUpdate || ImGui::DragFloat("Rotate angle", &_rotationAngle, 0.1,
                                         0, glm::pi<float>() * 2);
    ImGui::TreePop();
  }
  if (shouldUpdate)
    update();
}

void RotationSurface::drawGizmo() {
  glm::vec3 p1 = getPosition();
  p1 += glm::vec3{1, 0, 0} * _rotationRadius;
  auto p2 = p1 + glm::vec3{0, 0, 1};

  auto diff = p2 - p1;
  glm::vec3 xNorm(1.0, 0.0f, 0.0);
  glm::vec3 yNorm(0.0, 1.0f, 0.0);
  glm::vec3 zNorm(0.0, 0.0f, 1.0);

  auto _rotation = getRotation();

  diff = glm::rotate(diff, _rotation.x, xNorm); // Rotate on X axis
  diff = glm::rotate(diff, _rotation.y, yNorm); // Rotate on Y axis
  diff = glm::rotate(diff, _rotation.z, zNorm); // Rotate

  p2 = p1 + diff;

  ImGuizmo::DrawArrow({p1.x, p1.y, p1.z, 0}, {p2.x, p2.y, p2.z, 0}, 0xFF110055);
}

} // namespace EGEOM
