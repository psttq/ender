#pragma once

#include "Spline1.hpp"
namespace EGEOM {
class Wire : public ENDER::Object {
  using Edge = Spline1;
  std::vector<sptr<Edge>> _edges;
  Wire();

public:
  int currentEdge = 0;

  static sptr<Wire> create();
  void addEdge(sptr<Edge> edge);
  void removeEdge(sptr<Edge> edge);

  void setCurrentEdge(sptr<Edge> edge);
  bool isCurrentEdge(sptr<Edge> edge);

  void update();

  std::vector<sptr<Edge>> getEdges();
  sptr<Edge> getCurrentEdge();
};
} // namespace EGEOM
