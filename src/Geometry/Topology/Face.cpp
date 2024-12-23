#include "PlaneSurface.hpp"
#include "Surface.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "imgui.h"
#include <Topology/Face.hpp>
#include <Utilities.hpp>
#include <memory>

namespace EGEOM {
Face::Face(sptr<Surface> surface, sptr<Wire> wire, bool basedOnSurface)
    : ENDER::Object("Face"), _surface(surface), _wire(wire),
      _basedOnSurface(basedOnSurface) {
  type = ObjectType::Empty;
  addChildObject(wire);
  if (!basedOnSurface)
    addChildObject(surface);
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
sptr<Surface> Face::getSurface() { return _surface; }

void Face::addEdge(sptr<Edge> edge) { _wire->addEdge(edge); }

void Face::insertAfterEdge(sptr<Edge> edgeToInsert, sptr<Edge> edgeAfter){
    _wire->insertAfterEdge(edgeToInsert, edgeAfter);
}

void Face::setBasedOnSurface(bool isBasedOnSurface) {
  _basedOnSurface = isBasedOnSurface;
  if (_basedOnSurface)
    deleteChildObject(_surface);
  else
    addChildObject(_surface);
  update();
}

// Вычисляем нормаль к параметрической поверхности в точке
glm::vec3 calculateNormal(glm::vec3 p_u, glm::vec3 p_v) {
  return glm::normalize(glm::cross(p_u, p_v)); // Векторное произведение
}


// Проецируем 3D-точку на плоскость, заданную касательными векторами
glm::vec2 projectToPlane(const glm::vec3 &point, const glm::vec3 &origin,
                         const glm::vec3 &tangentX, const glm::vec3 &tangentY) {
  glm::vec3 relativePos =
      point - origin; // Перенос точки относительно начала координат
  return glm::vec2(glm::dot(relativePos, tangentX),
                   glm::dot(relativePos, tangentY)); // Координаты в плоскости
}

bool isIntersecting(const glm::vec2 &point, const glm::vec2 &p1,
                    const glm::vec2 &p2) {
  if ((p1.y > point.y) != (p2.y > point.y)) {
    double intersectX = p1.x + (point.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
    return point.x < intersectX || glm::abs(point.x - intersectX) < 1e-5;
  }
  return false;
}

bool isPointInsideCurve(const glm::vec2 &point,
                        const std::vector<glm::vec2> &curve) {
  int intersections = 0;
  for (size_t i = 0; i < curve.size(); ++i) {
    glm::vec2 p1 = curve[i];
    glm::vec2 p2 = curve[(i + 1) % curve.size()];
    if (isIntersecting(point, p1, p2)) {
      intersections++;
    }
  }
  return intersections % 2 != 0; // Нечетное число пересечений — точка внутри
}

void Face::update() {
  _wire->update();
  if (_basedOnSurface) {
    std::vector<glm::vec3> wirePoints;
    float step = 1.0 / (wirePointNumber - 1);
    for (auto i = 0; i < wirePointNumber; ++i) {
      wirePoints.push_back(_wire->getPoint(step * i));
    }

    if (_surface->getName() == "Plane") {

      sptr<PlaneSurface> plane =
          std::dynamic_pointer_cast<PlaneSurface>(_surface);
      if (!plane) {
        spdlog::error(
            "Face::update(): Something went wrong during cast to plane");
        return;
      }

      auto [pivot, b1, b2] = plane->getBasis();

      glm::vec2 minValues(std::numeric_limits<float>::max());
      glm::vec2 maxValues(std::numeric_limits<float>::lowest());

      for (const auto &point : wirePoints) { // BORDERS
        auto p = point - pivot;
        auto pr1 = glm::dot(b1, p);
        auto pr2 = glm::dot(b2, p);
        minValues.x = std::min(minValues.x, pr1);
        minValues.y = std::min(minValues.y, pr2);

        maxValues.x = std::max(maxValues.x, pr1);
        maxValues.y = std::max(maxValues.y, pr2);
      }

      pivot = pivot + minValues.x * b1 + minValues.y * b2;
      plane->setPivot(pivot);
      _surface->setUMinMax(0, maxValues.x - minValues.x);
      _surface->setVMinMax(0, maxValues.y - minValues.y);
    }

    auto normal = _surface->normalOnSurface(
        0.4, 0.4); // Actually wont work for some sophisticated surfaces, but
                   // for my purposes is okay
    glm::vec3 tangentX, tangentY;

    auto origin = _surface->pointOnSurface(0.4, 0.4);
    auto p = _surface->pointOnSurface(0.3, 0.4);

    tangentX = glm::normalize(p-origin);
    tangentY = glm::cross(tangentX, normal);



    std::vector<glm::vec2> projectedCurve;

    for (const auto &curvePoint : wirePoints) {
      auto proj = projectToPlane(curvePoint, origin, tangentX, tangentY);
      projectedCurve.push_back(proj);
    }

    std::vector<glm::vec3> surfacePoints;
    // auto [u_min, u_max, v_min, v_max] = _surface->getUVMinMax();
    auto u_max = 1.0f;
    auto u_min = 0.0f;
    auto v_max = 1.0f;
    auto v_min = 0.0f;

    float u_step = (u_max - u_min) / (surfacePointsNumberByU - 1);
    float v_step = (v_max - v_min) / (surfacePointsNumberByV - 1);

    for (auto i = 0; i < surfacePointsNumberByU; i++) {
      float u = u_min + u_step * i;
      for (auto j = 0; j < surfacePointsNumberByV; j++) {
        float v = v_min + v_step * j;
        auto point = _surface->pointOnSurface(u, v);
        auto point2D = projectToPlane(point, origin, tangentX, tangentY);
        if (isPointInsideCurve(point2D, projectedCurve)) {
            surfacePoints.push_back(point);
            }
      }
    }
    spdlog::error("NUM: {}", surfacePoints.size());

    auto vao =
        ENDER::Utils::createTriangulationSurfaceVAO(wirePoints, surfacePoints);
    setVertexArray(vao);
    type = ObjectType::Surface;
  } else {
    _surface->update();
    type = ObjectType::Empty;
  }
}

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

  _surface->drawGizmo();
}
} // namespace EGEOM
