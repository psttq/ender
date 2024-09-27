#include <Geometry/Topology/Wire.hpp>

namespace EGEOM
{
    Wire::Wire() : Object("Wire")
    {
        type = ObjectType::Empty;
    }
    sptr<Wire> Wire::create() { return sptr<Wire>(new Wire()); }
    void Wire::addEdge(sptr<Edge> edge)
    {
        _edges.push_back(edge);
        addChildObject(edge);
    }
    void Wire::removeEdge(sptr<Edge> edge) { _edges.push_back(edge); }

    void Wire::setCurrentEdge(sptr<Edge> edge)
    {
        for (int i = 0; i < _edges.size(); i++)
        {
            if (_edges[i] == edge)
            {
                currentEdge = i;
                break;
            }
        }
    }

    void Wire::update()
    {
        for (auto &edge : _edges)
        {
            edge->update();
        }
    }

    bool Wire::isCurrentEdge(sptr<Edge> edge)
    {
        return _edges[currentEdge] == edge;
    }

    std::vector<sptr<Edge>> Wire::getEdges() { return _edges; }
    sptr<Edge> Wire::getCurrentEdge() { return _edges[currentEdge]; }
} // namespace EGEOM
