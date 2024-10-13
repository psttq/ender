#include "Object.hpp"
#include "Surface.hpp"
#include "Utilities.hpp"
#include "glm/geometric.hpp"
#include "imgui.h"
#include <ExtrudeSurface.hpp>
#include <ImGuizmo.h>
#include <glm/gtx/rotate_vector.hpp>

namespace EGEOM
{
  ExtrudeSurface::ExtrudeSurface(const std::string &name,
                                 sptr<Spline1> baseSpline,
                                 const glm::vec3 &direction, float extrudeLength)
      : Surface(name), _baseSpline(baseSpline), _direction(direction),
        _length(extrudeLength)
  {
    update();
  }

  sptr<ExtrudeSurface> ExtrudeSurface::create(const std::string &name,
                                              sptr<Spline1> baseSpline,
                                              const glm::vec3 &direction,
                                              float extrudeLength)
  {
    return sptr<ExtrudeSurface>(
        new ExtrudeSurface(name, baseSpline, direction, extrudeLength));
  }

  void ExtrudeSurface::update()
  {
    auto vao = ENDER::Utils::createParametricSurfaceVAO(
        [&](float u, float v)
        { return pointOnSurface(u, v); }, 0, 0, 1, 1,
        SURFACE_ROWS, SURFACE_COLS);
    setVertexArray(vao);
  }

  glm::vec3 ExtrudeSurface::pointOnSurface(float u, float v)
  {
    auto direction = _direction / glm::length(_direction);
    return _baseSpline->getSplinePoint(u) + v * _length * direction;
  }

  glm::vec3 ExtrudeSurface::normalOnSurface(float u, float v)
  {
    auto direction = _direction / glm::length(_direction);

    glm::vec3 cdu = _baseSpline->getSplineDirs(u, 2)[1]->getPosition();
    glm::vec3 cdv = _length * direction;
    auto normal = glm::normalize(glm::cross(cdu, cdv));
    return normal;
  }

  void ExtrudeSurface::drawProperties()
  {
    ENDER::Object::drawProperties();
    bool shouldUpdate = false;
    if (ImGui::TreeNode("Extrude Surface"))
    {
      shouldUpdate = ImGui::DragFloat3("Extrude Direction",
                                       glm::value_ptr(_direction), 0.1f, -1, 1);
      shouldUpdate =
          shouldUpdate || ImGui::DragFloat("Extrude Height", &_length, 0.1);
      ImGui::TreePop();
    }
    if (shouldUpdate)
      update();
  }

  void ExtrudeSurface::drawGizmo()
  {
    auto direction = _direction / glm::length(_direction);

    glm::vec3 p1 = getPosition();
    auto p2 = p1 + direction * _length;

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

    int n = 5;
    int m = 5;

    auto u_step = 1.0 / (n - 1);
    auto v_step = 1.0 / (m - 1);

    for (auto i = 0; i < n; i++)
    {
      auto u = i * u_step;
      for (auto j = 0; j < m; j++)
      {
        auto v = j * v_step;
        auto pc = pointOnSurface(u, v);
        auto pn = normalOnSurface(u, v) + pc;
        ImGuizmo::DrawArrow({pc.x, pc.y, pc.z, 0}, {pn.x, pn.y, pn.z, 0}, 0xFFFFFF55);
      }
    }
  }

} // namespace EGEOM
