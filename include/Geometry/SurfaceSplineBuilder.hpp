#pragma once

#include "Spline1.hpp"
#include "SplineBuilder.hpp"
#include "Surface.hpp"

namespace EGEOM {

class SurfaceSplineBuilder : public SplineBuilder {
  sptr<Surface> _r;

  sptr<Spline1> _u;
  sptr<Spline1> _v;

public:
  SurfaceSplineBuilder(sptr<Surface> r, sptr<Spline1> u, sptr<Spline1> v);

  void rebuild() override;

  sptr<Point> getSplinePoint(float t) override;

  bool drawPropertiesGui() override;

  uptr<SplineBuilder> clone() override;
};

} // namespace EGEOM
