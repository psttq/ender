#pragma once

#include "Spline1.hpp"
namespace EGEOM {
class Wire {
  using Edge = Spline1;
  std::vector<sptr<Edge>> _edges;
  Wire() = default;

public:
  static sptr<Wire> create();
  void addEdge(sptr<Edge> edge);
  void removeEdge(sptr<Edge> edge);
  std::vector<sptr<Edge>> getEdges();
};
} // namespace EGEOM
