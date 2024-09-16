#pragma once

#include "Object.hpp"
#include "Spline1.hpp"
#include "Surface.hpp"
namespace EGEOM {
class Face : public ENDER::Object {
  using Edge = Spline1;

  sptr<Surface> _surface;
  std::vector<sptr<Edge>> _edges;
  Face(sptr<Surface> surface, std::vector<sptr<Edge>> edges);

public:
  static sptr<Face> create(sptr<Surface> surface,
                           std::vector<sptr<Edge>> edges);

  void addEdge(sptr<Edge> edge);

  void drawGizmo() override;
};
} // namespace EGEOM
