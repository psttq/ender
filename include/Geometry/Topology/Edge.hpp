#pragma once

#include "Object.hpp"
#include "Spline1.hpp"

namespace EGEOM {
class Edge : public ENDER::Object {
  sptr<Spline1> _spline;
  Edge(sptr<Spline1> spline);

public:
  bool isInvertedDirection = false;

  ImU32 gizmoColor = 0x44FF44FF;
public:
  static sptr<Edge> create(sptr<Spline1> spline);

  sptr<Spline1> getSpline() const;

  sptr<Edge> clone();
  sptr<Edge> copy();

  void update();

  void drawGizmo() override;

  void drawProperties() override;
};
} // namespace EGEOM
