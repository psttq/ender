#pragma once

#include "Spline1.hpp"
#include "Surface.hpp"
#include "glm/fwd.hpp"
#include <Topology/Edge.hpp>

namespace EGEOM{

#define MAX_ITERS 100
#define SPLINE_APPROX_POINTS 100

class FilletSurface: public Surface{
    sptr<Edge> _edge;

    sptr<Surface> _leftSurface;
    sptr<Surface> _rightSurface;

    sptr<Spline1> u_spline;
    sptr<Spline1> v_spline;
    sptr<Spline1> a_spline;
    sptr<Spline1> b_spline;

    float lR = 1.0f;
    float rR = 1.0f;

    FilletSurface(sptr<Edge> edge, sptr<Surface> leftSurface, sptr<Surface> rightSurface);

    glm::vec4 _equationSystem(float u, float v, float a, float b, float s);

    glm::mat4 _jacobian(float u, float v, float a, float b, float s);

    glm::vec4 _newtonMethod(float u, float v, float a, float b, float s);

    public:
    static sptr<FilletSurface> create(sptr<Edge> edge, sptr<Surface> leftSurface, sptr<Surface> rightSurface);

    void update() override;

};
}
