#include "Surface.hpp"
#include <SurfaceSplineBuilder.hpp>
namespace EGEOM {
SurfaceSplineBuilder::SurfaceSplineBuilder(sptr<Surface> r, sptr<Spline1> u,
                                           sptr<Spline1> v)
    : SplineBuilder({}), _r(r), _u(u), _v(v) {}

void SurfaceSplineBuilder::rebuild() {}

sptr<Point> SurfaceSplineBuilder::getSplinePoint(float t) {
  auto u = _u->getSplinePoint(t);
  auto v = _v->getSplinePoint(t);
  auto point = _r->pointOnSurface(u.z, v.z);
  return Point::create(point);
}

bool SurfaceSplineBuilder::drawPropertiesGui() { return false; }

uptr<SplineBuilder> SurfaceSplineBuilder::clone() { return {}; }

} // namespace EGEOM
