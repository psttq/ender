#pragma once

#include "Spline1.hpp"
#include "Surface.hpp"
namespace EGEOM {

class KinematicSurface : public Surface {
public:
  enum class KinematicSurfaceType { Sweep, Shift };

private:
  sptr<Spline1> _formingSpline;
  sptr<Spline1> _guideSpline;
  KinematicSurfaceType _type;

  std::vector<sptr<Point>> g0;
  glm::mat3 Am;

  KinematicSurface(const std::string &name, const sptr<Spline1> &formingSpline,
                   const sptr<Spline1> &guideSpline,
                   const KinematicSurfaceType &type);

  glm::vec3 sweepSplineHelper(float v);

public:
  static sptr<KinematicSurface> create(const std::string &name,
                                       const sptr<Spline1> &formingSpline,
                                       const sptr<Spline1> &guideSpline,
                                       const KinematicSurfaceType &type);

  glm::vec3 pointOnSurface(float u, float v) override;

  void drawProperties() override;
  void drawGizmo() override;

  void update() override;
};

} // namespace EGEOM
