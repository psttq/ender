#include "Spline1.hpp"
#include "SplineBuilder.hpp"
#include <FilletSurface.hpp>

namespace EGEOM {

glm::vec4 FilletSurface::_equationSystem(float u, float v, float a, float b,
                                         float s) {
  auto r = _leftSurface->pointOnSurface(u, v);
  auto sp = _rightSurface->pointOnSurface(a, b);

  auto mr = _leftSurface->normalOnSurface(u, v);
  auto ms = _rightSurface->normalOnSurface(a, b);

  auto residual1 = r + lR * mr - sp - rR * ms;

  auto c0dirs = _edge->getEdgeDirs(s, 2);

  auto c0 = c0dirs[0]->getPosition();
  auto c0ds = c0dirs[1]->getPosition();

  auto residual2 = glm::dot((c0 - 0.5f * (r + lR * mr + sp + rR * ms)), c0ds);

  return glm::vec4(residual1, residual2);
}

glm::mat4 FilletSurface::_jacobian(float u, float v, float a, float b,
                                   float s) {
  float delta = 1e-5;

  auto eq = _equationSystem(u, v, a, b, s);

  auto eq_delta = _equationSystem(u + delta, v, a, b, s);
  auto u_dir = (eq_delta - eq) / delta;

  eq_delta = _equationSystem(u, v + delta, a, b, s);
  auto v_dir = (eq_delta - eq) / delta;

  eq_delta = _equationSystem(u, v, a + delta, b, s);
  auto a_dir = (eq_delta - eq) / delta;

  eq_delta = _equationSystem(u, v, a, b + delta, s);
  auto b_dir = (eq_delta - eq) / delta;

  return {u_dir, v_dir, a_dir, b_dir};
}

glm::vec4 FilletSurface::_newtonMethod(float u, float v, float a, float b,
                                       float s) {
  float eps = 1e-5;
  int max_iter = 100;

  float tol = 100;
  for (auto i = 0; i < MAX_ITERS || tol > eps; i++) {
    auto F = _equationSystem(u, v, a, b, s);
    auto J = _jacobian(u, v, a, b, s);
    auto J_inv = glm::inverse(J);

    auto delta = J_inv * F;

    u = -delta.x;
    v = -delta.y;
    a = -delta.z;
    b = -delta.w;
  }

  return {u, v, a, b};
}

void FilletSurface::update() {
  const int s_step = 1.0 / (SPLINE_APPROX_POINTS - 1);
  std::vector<float> s;

  std::vector<float> u_approx;
  std::vector<float> v_approx;
  std::vector<float> a_approx;
  std::vector<float> b_approx;

  glm::vec4 initialGuess = {0, 0, 0, 0};
  for (auto i = 0; i < SPLINE_APPROX_POINTS; i++) {
    auto approx = _newtonMethod(initialGuess.x, initialGuess.y, initialGuess.z,
                                initialGuess.w, i * s_step);

    u_approx.push_back(approx.x);
    v_approx.push_back(approx.y);
    a_approx.push_back(approx.z);
    b_approx.push_back(approx.w);

    s.push_back(i * s_step);
    initialGuess = approx;
  }

  auto createSpline = [](std::vector<float> x, std::vector<float> y) {
    auto spline = Spline1::create({}, 2);
    auto cubic_spline_builder =
        uptr<SplineBuilder>(new CubicSplineBuilder(x, y));
    spline->setSplineType(Spline1::SplineType::CubicSpline);
    spline->setSplineBuilder(std::move(cubic_spline_builder));
    return spline;
  };

  u_spline = createSpline(s, u_approx);
  v_spline = createSpline(s, v_approx);
  a_spline = createSpline(s, a_approx);
  b_spline = createSpline(s, b_approx);
}
} // namespace EGEOM
