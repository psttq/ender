#pragma once

#include "Spline1.hpp"
#include "Surface.hpp"
namespace EGEOM {

class ExtrudeSurface : public Surface {
  sptr<Spline1> _baseSpline;
  glm::vec3 _direction;
  float _length;
  ExtrudeSurface(const std::string &name, sptr<Spline1> baseSpline,
                 const glm::vec3 &direction, float extrudeLength);

public:
  static sptr<ExtrudeSurface> create(const std::string &name,
                                     sptr<Spline1> baseSpline,
                                     const glm::vec3 &direction,
                                     float extrudeLength);

  glm::vec3 pointOnSurface(float u, float v) override;

  void drawProperties() override;
  void drawGizmo() override;

  void update();
};

} // namespace EGEOM
