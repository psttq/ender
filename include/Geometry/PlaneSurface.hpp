#pragma once

#include "Surface.hpp"
namespace EGEOM {
class PlaneSurface : public EGEOM::Surface {
  PlaneSurface(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c);

  glm::vec3 _pivot;
  glm::vec3 _basisVec1;
  glm::vec3 _basisVec2;

  glm::vec3 _normal;

public:
  static sptr<PlaneSurface> create(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c);

  void update() override;

  std::tuple<glm::vec3, glm::vec3, glm::vec3> getBasis();
  void setPivot(const glm::vec3 &pivot);

  void drawProperties() override;
  void drawGizmo() override;

  glm::vec3 pointOnSurface(float u, float v) override;
  glm::vec3 normalOnSurface(float u, float v) override;
  std::tuple<glm::vec3,glm::vec3> firstDir(float u, float v) override;

};
} // namespace EGEOM
