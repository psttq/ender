#include <Face.hpp>

namespace EGEOM
{
  Face::Face(sptr<Surface> surface, std::vector<sptr<Edge>> edges)
      : ENDER::Object("Face"), _surface(surface), _edges(edges)
  {
    type = ObjectType::Empty;
    addChildObject(surface);
    for (auto edge : _edges)
    {
      addChildObject(edge);
    }
  }

  sptr<Face> Face::create(sptr<Surface> surface, std::vector<sptr<Edge>> edges)
  {
    return sptr<Face>(new Face(surface, edges));
  }

  void Face::addEdge(sptr<Edge> edge)
  {
    _edges.push_back(edge);
    addChildObject(edge);
  }
} // namespace EGEOM
