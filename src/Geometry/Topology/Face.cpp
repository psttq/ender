#include "glm/fwd.hpp"
#include "imgui.h"
#include <Topology/Face.hpp>
#include <Utilities.hpp>

namespace EGEOM
{
  Face::Face(sptr<Surface> surface, sptr<Wire> wire, bool basedOnSurface)
      : ENDER::Object("Face"), _surface(surface), _wire(wire),
        _basedOnSurface(basedOnSurface)
  {
    type = ObjectType::Empty;
    addChildObject(wire);
    // if (!basedOnSurface)
      // addChildObject(surface);
  }

  sptr<Face> Face::create(sptr<Surface> surface, sptr<Wire> wire)
  {
    return sptr<Face>(new Face(surface, wire));
  }

  void Face::setWire(sptr<Wire> wire)
  {
    deleteChildObject(_wire);
    _wire = wire;
    addChildObject(_wire);
  }

  sptr<Wire> Face::getWire() { return _wire; }
  sptr<Surface> Face::getSurface() { return _surface; }

  void Face::addEdge(sptr<Edge> edge) { _wire->addEdge(edge); }

  void Face::setBasedOnSurface(bool isBasedOnSurface)
  {
    _basedOnSurface = isBasedOnSurface;
    // if(_basedOnSurface)
      // deleteChildObject(_surface);
    // else
      // addChildObject(_surface);
    update();
  }

  void Face::update()
  {
    _wire->update();
    if (_basedOnSurface)
    {
      std::vector<glm::vec3> wirePoints;
      float step = 1.0 / (wirePointNumber - 1);
      for (auto i = 0; i < wirePointNumber; ++i)
      {
        wirePoints.push_back(_wire->getPoint(step * i));
      }

      // glm::vec3 minValues(std::numeric_limits<float>::max());
      // glm::vec3 maxValues(std::numeric_limits<float>::lowest());

      // for (const auto &point : wirePoints) { // BORDERS
      //   minValues.x = std::min(minValues.x, point.x);
      //   minValues.y = std::min(minValues.y, point.y);
      //   minValues.z = std::min(minValues.z, point.z);

      //   maxValues.x = std::max(maxValues.x, point.x);
      //   maxValues.y = std::max(maxValues.y, point.y);
      //   maxValues.z = std::max(maxValues.z, point.z);
      // }
      if (_surface->getName() == "Plane")
      {
      }

      std::vector<glm::vec3> surfacePoints;
      auto [u_min, u_max, v_min, v_max] = _surface->getUVMinMax();

      float u_step = (u_max - u_min) / (surfacePointsNumberByU - 1);
      float v_step = (v_max - v_min) / (surfacePointsNumberByV - 1);

      for (auto i = 0; i < surfacePointsNumberByU; i++)
      {
        float u = u_min + u_step * i;
        for (auto j = 0; j < surfacePointsNumberByV; j++)
        {
          float v = v_min + v_step * j;
          auto point = _surface->pointOnSurface(u, v);
          bool inside = false;
          for (auto k = 0; k < wirePointNumber;
               k++)
          { // FIXME: WORKS ONLY FOR 2D XZ PLANE!!!
            size_t kn = (k + 1) % wirePointNumber;
            auto pk = wirePoints[k];
            auto pkn = wirePoints[kn];
            bool intersect = ((pk.z > point.z) != (pkn.z > point.z)) &&
                             (point.x < (pkn.x - pk.x) * (point.z - pk.z) /
                                                (pkn.z - pk.z + 1e-9) +
                                            pk.x);

            if (intersect)
            {
              inside = !inside;
            }
          }
          if (inside)
          {
            surfacePoints.push_back(point);
          }
        }
      }
      for (auto wirePoints : wirePoints)
      {
        surfacePoints.push_back(wirePoints);
      }
      auto vao = ENDER::Utils::createTriangulationSurfaceVAO(surfacePoints);
      setVertexArray(vao);
      type = ObjectType::Surface;
    }
    else
    {
      _surface->update();
      type = ObjectType::Empty;
    }
  }

  void Face::drawGizmo()
  {
    glm::vec3 color1 = {1, 0, 0};
    glm::vec3 color2 = {0, 0, 1};
    auto edges = _wire->getEdges();
    auto step = 1.0f / (edges.size() - 1);
    auto i = 0;
    for (auto edge : edges)
    {
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
