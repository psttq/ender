#include <Wire.hpp>

namespace EGEOM {
sptr<Wire> Wire::create() { return sptr<Wire>(new Wire()); }
void Wire::addEdge(sptr<Edge> edge) { _edges.push_back(edge); }
void Wire::removeEdge(sptr<Edge> edge) { _edges.push_back(edge); }

std::vector<sptr<Wire::Edge>> Wire::getEdges() { return _edges; }
} // namespace EGEOM
