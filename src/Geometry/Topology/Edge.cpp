#include <Topology/Edge.hpp>

#include "ImGuizmo.h"

namespace EGEOM {
    Edge::Edge(sptr<Spline1> spline) : ENDER::Object("Edge"), _spline(spline){
        type = ObjectType::Empty;
        addChildObject(spline);
    }

    sptr<Edge> Edge::create(sptr<Spline1> spline){
        return sptr<Edge>(new Edge(spline));
    }

    void Edge::update(){
        _spline->update();
    }

    sptr<Spline1> Edge::getSpline() const{
        return _spline;
    }

    sptr<Edge> Edge::clone(){
        return Edge::create(_spline->clone());
    }

    sptr<Edge> Edge::copy(){
        return Edge::create(_spline);
    }

    void Edge::drawProperties(){

    }

    void Edge::drawGizmo(){
        auto points = _spline->getPoints();
        if (points.size() < 2)
          return;
        for (auto p1i = points.begin(), end = points.end(); p1i + 1 != end; ++p1i) {
          auto p2i = p1i + 1;
          if (p2i == end)
            break;

          auto p1 = p1i->get()->getPosition() + getPosition();
          auto p2 = p2i->get()->getPosition() + getPosition();
          if (isInvertedDirection) {
            auto tmp = p1;
            p1 = p2;
            p2 = tmp;
          }
          ImGuizmo::DrawArrow({p1.x, p1.y, p1.z, 0}, {p2.x, p2.y, p2.z, 0},
                              gizmoColor);
        }
    }
}
