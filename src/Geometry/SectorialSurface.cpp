#include "Utilities.hpp"
#include "spdlog/spdlog.h"
#include <SectorialSurface.hpp>

namespace EGEOM {

SectorialSurface::SectorialSurface(sptr<Point> point, sptr<Wire> wire)
    : Surface("SectorialSurface"), _point(point), _wire(wire) {
  update();
}
sptr<SectorialSurface> SectorialSurface::create(sptr<Point> point,
                                                sptr<Wire> wire) {
  return sptr<SectorialSurface>(new SectorialSurface(point, wire));
}
void SectorialSurface::update() {
  auto vao = ENDER::Utils::createParametricSurfaceVAO(
      [&](float u, float v) { return pointOnSurface(u, v); }, 0, 0, 1, 1,
      SURFACE_ROWS / 2, SURFACE_COLS / 2);
  setVertexArray(vao);
}
glm::vec3 SectorialSurface::pointOnSurface(float u, float v) {
  int number_of_edge = 1;
  auto edges = _wire->getEdges();
  auto edges_count = edges.size();
  float step = 1.0f / (float)edges_count;
  spdlog::error("step: {}", step);
  while (u > number_of_edge * step) {
    spdlog::info("u: {}, n: {}, c: {}", u, number_of_edge,
                 number_of_edge * step);
    number_of_edge++;
  }
  number_of_edge -= 1;
  spdlog::error("u: {}, v: {}", u, v);
  spdlog::error("Number of edge: {}", number_of_edge);

  float w = (u - number_of_edge * step) * edges_count;
  spdlog::error("w: {}", w);

  auto point_on_surface = (1 - v) * edges[number_of_edge]->getSplinePoint(w) +
                          v * (_point->getPosition() + glm::vec3{0, 1, 0});

  spdlog::error("Point: {} {} {}", point_on_surface.x, point_on_surface.y,
                point_on_surface.z);
  return point_on_surface;
}
void SectorialSurface::drawProperties() {
  ENDER::Object::drawProperties();
  bool shouldUpdate = false;
  if (ImGui::TreeNode("Sectorial Surface")) {
    shouldUpdate = ImGui::DragFloat3(
        "Point", glm::value_ptr(_point->getPosition()), 0.1f, -1, 1);
    ImGui::TreePop();
  }
  if (shouldUpdate)
    update();
}
} // namespace EGEOM
