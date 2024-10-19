#pragma once

#include "Object.hpp"
#include "Surface.hpp"
#include "Wire.hpp"

namespace EGEOM {
class Face : public ENDER::Object {

  const int wirePointNumber = 100;
  const int surfacePointsNumberByU = 100;
  const int surfacePointsNumberByV = 100;

  sptr<Surface> _surface;
  sptr<Wire> _wire;

  bool _basedOnSurface = false;

  Face(sptr<Surface> surface, sptr<Wire> wire, bool basedOnSurface = false);

public:
  static sptr<Face> create(sptr<Surface> surface, sptr<Wire> wire);

  void setWire(sptr<Wire> wire);
  sptr<Wire> getWire();
  sptr<Surface> getSurface();

  void setBasedOnSurface(bool isBasedOnSurface);

  void addEdge(sptr<Edge> edge);

  void update();

  void drawGizmo() override;
};
} // namespace EGEOM
