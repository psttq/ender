#include "Surface.hpp"
#include <SurfaceSplineBuilder.hpp>
namespace EGEOM {
SurfaceSplineBuilder::SurfaceSplineBuilder(sptr<Surface> r, sptr<Spline1> u,
                                           sptr<Spline1> v)
    : SplineBuilder({}), _r(r), _u(u), _v(v) {}

void SurfaceSplineBuilder::rebuild() {}

sptr<Point> SurfaceSplineBuilder::_getSplinePoint(float t) {
  auto u = _u->getSplinePoint(t);
  auto v = _v->getSplinePoint(t);
  auto point = _r->pointOnSurface(u.z, v.z);
  return Point::create(point);
}

std::vector<sptr<Point>>
SurfaceSplineBuilder::getSplineDerivatives(float t, int dirsCount) {
  auto udts = _u->getSplineDirs(t, 2);
  auto vdts = _v->getSplineDirs(t, 2);
  auto [cdu, cdv] =
      _r->firstDir(udts[0]->getPosition().z, vdts[0]->getPosition().z);
  auto firstDir = cdu * udts[1]->getPosition().z + cdv * vdts[1]->getPosition().z;
  return {getSplinePoint(t), Point::create(firstDir)};
}

bool SurfaceSplineBuilder::drawPropertiesGui() { return false; }

uptr<SplineBuilder> SurfaceSplineBuilder::clone() { return {}; }

} // namespace EGEOM
