#pragma once

#include "Point.hpp"
#include "Surface.hpp"
#include "Topology/Wire.hpp"

namespace EGEOM {
class SectorialSurface : public EGEOM::Surface {
  sptr<Point> _point;
  sptr<Wire> _wire;

  SectorialSurface(sptr<Point> point, sptr<Wire> wire);

public:
  static sptr<SectorialSurface> create(sptr<Point> point, sptr<Wire> wire);

  void update();

  void drawProperties() override;

  glm::vec3 pointOnSurface(float u, float v) override;
};
} // namespace EGEOM
