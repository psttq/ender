#include "imgui.h"
#include <Topology/Face.hpp>

namespace EGEOM {
Face::Face(sptr<Surface> surface, sptr<Wire> wire)
    : ENDER::Object("Face"), _surface(surface), _wire(wire) {
  type = ObjectType::Empty;
  addChildObject(surface);
  addChildObject(wire);
}

sptr<Face> Face::create(sptr<Surface> surface, sptr<Wire> wire) {
  return sptr<Face>(new Face(surface, wire));
}

void Face::setWire(sptr<Wire> wire) {
  deleteChildObject(_wire);
  _wire = wire;
  addChildObject(_wire);
}

sptr<Wire> Face::getWire() { return _wire; }
sptr<Surface> Face::getSurface(){ return _surface; }

void Face::addEdge(sptr<Edge> edge) { _wire->addEdge(edge); }

void Face::drawGizmo() {
  glm::vec3 color1 = {1, 0, 0};
  glm::vec3 color2 = {0, 0, 1};
  auto edges = _wire->getEdges();
  auto step = 1.0f / (edges.size() - 1);
  auto i = 0;
  for (auto edge : edges) {
    auto fraction = step * i;
    auto new_color = (color2 - color1) * fraction + color1;
    auto color = ImGui::ColorConvertFloat4ToU32(
        {new_color.r, new_color.g, new_color.b, 1});
    edge->gizmoColor = color;
    edge->drawGizmo();
    i += 1;
  }
}
} // namespace EGEOM
