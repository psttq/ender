#pragma once

#include "Object.hpp"
#include "Spline1.hpp"
#include "Surface.hpp"
#include "Wire.hpp"

namespace EGEOM {
class Face : public ENDER::Object {
  using Edge = Spline1;

  sptr<Surface> _surface;
  sptr<Wire> _wire;
  Face(sptr<Surface> surface, sptr<Wire> wire);

public:
  static sptr<Face> create(sptr<Surface> surface, sptr<Wire> wire);

  void setWire(sptr<Wire> wire);
  sptr<Wire> getWire();

  void addEdge(sptr<Edge> edge);

  void drawGizmo() override;
};
} // namespace EGEOM
