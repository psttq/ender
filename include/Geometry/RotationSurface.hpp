#pragma once

#include "Spline1.hpp"
#include "Surface.hpp"
namespace EGEOM {

class RotationSurface : public Surface {

  sptr<Spline1> _baseSpline;
  float _rotationRadius = 1.0f;
  float _rotationAngle = glm::pi<float>() * 2;
  RotationSurface(const std::string &name, sptr<Spline1> baseSpline,
                  float rotationAngle, float rotationRadius);


public:
  static sptr<RotationSurface> create(const std::string &name, sptr<Spline1> baseSpline,
                  float rotationAngle, float rotationRadius);

  glm::vec3 pointOnSurface(float u, float v) override;

  void drawProperties() override;

  void update();
};

} // namespace EGEOM
