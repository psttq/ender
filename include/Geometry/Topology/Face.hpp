#pragma once

#include "Object.hpp"
#include "Surface.hpp"
#include "Wire.hpp"

namespace EGEOM {
class Face : public ENDER::Object {

  sptr<Surface> _surface;
  sptr<Wire> _wire;
  Face(sptr<Surface> surface, sptr<Wire> wire);

public:
  static sptr<Face> create(sptr<Surface> surface, sptr<Wire> wire);

  void setWire(sptr<Wire> wire);
  sptr<Wire> getWire();
  sptr<Surface> getSurface();

  void addEdge(sptr<Edge> edge);

  void drawGizmo() override;
};
} // namespace EGEOM
