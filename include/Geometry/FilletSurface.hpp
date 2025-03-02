#pragma once

#include "Spline1.hpp"
#include "Surface.hpp"
#include "glm/fwd.hpp"
#include <Topology/Edge.hpp>

namespace EGEOM {

#define MAX_ITERS 4000
#define SPLINE_APPROX_POINTS 1000

class FilletSurface : public Surface {
  sptr<Edge> _edge;

  sptr<Surface> _leftSurface;
  sptr<Surface> _rightSurface;

  sptr<Spline1> u_spline;
  sptr<Spline1> v_spline;
  sptr<Spline1> a_spline;
  sptr<Spline1> b_spline;

  sptr<Spline1> cr;
  sptr<Spline1> cs;

  // float lR = 0.4f;
  // float rR = 0.4f;

  float lR(float s) {
    return static_cast<float>(0.4 +
     0.5 * glm::sin(s * glm::pi<float>() / 2.0f));

    return 0.4f;
  }

  float rR(float s) {
    return static_cast<float>(0.4 +
    0.5 * glm::sin(s * glm::pi<float>() / 2.0f));
    return 0.4f;
  }

  FilletSurface(sptr<Edge> edge, sptr<Surface> leftSurface,
                sptr<Surface> rightSurface);

  glm::vec4 _equationSystem(float u, float v, float a, float b, float s);

  glm::mat4 _jacobian(float u, float v, float a, float b, float s);

  glm::vec4 _newtonMethod(float u, float v, float a, float b, float s);

  glm::vec4 _findInitialGuess(float s);

public:
  static sptr<FilletSurface> create(sptr<Edge> edge, sptr<Surface> leftSurface,
                                    sptr<Surface> rightSurface);

  sptr<Spline1> getCrSpline();
  sptr<Spline1> getCsSpline();

  void update() override;

  glm::vec3 pointOnSurface(float u, float v) override;
};
} // namespace EGEOM
