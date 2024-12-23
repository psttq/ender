#include "Spline1.hpp"
#include "SplineBuilder.hpp"
#include "Surface.hpp"
#include "SurfaceSplineBuilder.hpp"
#include "Utilities.hpp"
#include <FilletSurface.hpp>

namespace EGEOM {

FilletSurface::FilletSurface(sptr<Edge> edge, sptr<Surface> leftSurface,
                             sptr<Surface> rightSurface)
    : Surface("Fillet"), _edge(edge), _leftSurface(leftSurface),
      _rightSurface(rightSurface) {}

sptr<FilletSurface> FilletSurface::create(sptr<Edge> edge,
                                          sptr<Surface> leftSurface,
                                          sptr<Surface> rightSurface) {
  return sptr<FilletSurface>(
      new FilletSurface(edge, leftSurface, rightSurface));
}
glm::vec4 FilletSurface::_equationSystem(float u, float v, float a, float b,
                                         float s) {
  auto r = _leftSurface->pointOnSurface(u, v);
  auto sp = _rightSurface->pointOnSurface(a, b);

  auto mr = _leftSurface->normalOnSurface(u, v);
  auto ms = _rightSurface->normalOnSurface(a, b);

  auto residual1 = r + lR * mr - sp - rR * ms;

  auto c0dirs = _edge->getEdgeDirs(s, 2);

  auto c0 = c0dirs[0]->getPosition();
  auto c0ds = -glm::normalize(c0dirs[1]->getPosition());

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
  float eps = 1e-7;

  float tol = 100;
  float step = 0.01;
  int i = 0;

  // auto [u_min, u_max, v_min, v_max] = _leftSurface->getUVMinMax();
  // auto [a_min, a_max, b_min, b_max] = _rightSurface->getUVMinMax();
  auto u_min = 0;
  auto u_max = 1;
  auto v_min = 0;
  auto v_max = 1;

  auto a_min = 0;
  auto a_max = 1;
  auto b_min = 0;
  auto b_max = 1;

  // spdlog::info("Starting newton method. U_MAX: {}, U_MIN: {}, V_MAX: {},
  // V_MIN: {}", u_max, u_min, v_max, v_min);;
  for (i = 0; i < MAX_ITERS && tol > eps; i++) {
    if (u < u_min)
      u = u_min;
    if (v < v_min)
      v = v_min;
    if (a < a_min)
      a = a_min;
    if (b < b_min)
      b = b_min;

    if (u > u_max)
      u = u_max;
    if (v > v_max)
      v = v_max;
    if (a > a_max)
      a = a_max;
    if (b > b_max)
      b = b_max;
    auto F = _equationSystem(u, v, a, b, s);
    auto J = _jacobian(u, v, a, b, s);
    auto Jdet = glm::determinant(J);

    if (Jdet == 0) {
      spdlog::error("FilletSurface: initial guess gives bad jacobian.");
      return {};
    }

    auto J_inv = glm::inverse(J);

    auto delta = step * J_inv * F;

    tol = glm::length(
        glm::vec4(u, v, a, b) -
        glm::vec4(u - delta.x, v - delta.y, a - delta.z, b - delta.w));

    u -= delta.x;
    v -= delta.y;
    a -= delta.z;
    b -= delta.w;

    if (u < u_min)
      u = u_min;
    if (v < v_min)
      v = v_min;
    if (a < a_min)
      a = a_min;
    if (b < b_min)
      b = b_min;

    if (u > u_max)
      u = u_max;
    if (v > v_max)
      v = v_max;
    if (a > a_max)
      a = a_max;
    if (b > b_max)
      b = b_max;
  }
  // spdlog::info("iter: {}, tolerance: {}", i, tol);
  // spdlog::info("u_i = {}, v_i = {}, a_i = {}, b_i = {}, s = {}", u, v, a, b,
  // s);

  return {u, v, a, b};
}

void FilletSurface::update() {
  const float s_step = 1.0 / (SPLINE_APPROX_POINTS - 1);
  std::vector<float> s;

  std::vector<float> u_approx;
  std::vector<float> v_approx;
  std::vector<float> a_approx;
  std::vector<float> b_approx;

  glm::vec4 initialGuess = {0.3, 0.4, 0.5, 0.7};
  spdlog::info("Starting fillet surface creation.");
  for (auto i = 0; i < SPLINE_APPROX_POINTS; i++) {
    auto approx = _newtonMethod(initialGuess.x, initialGuess.y, initialGuess.z,
                                initialGuess.w, i * s_step);

    u_approx.push_back(approx.x);
    v_approx.push_back(approx.y);
    a_approx.push_back(approx.z);
    b_approx.push_back(approx.w);

    auto cur_pers = (int)((float)i / ((float)SPLINE_APPROX_POINTS) * 100.0f);
    if(cur_pers%10 == 0)
        spdlog::info("{}% completed", cur_pers);
        
    s.push_back(i * s_step);
    initialGuess = approx;
  }

  // for(auto i = 0; i < a_approx.size(); i++){
  //   spdlog::info("ab: {} {}", a_approx[i], b_approx[i]);
  // }
  //  for(auto i = 0; i < u_approx.size(); i++){
  //   spdlog::info("uv: {} {}", u_approx[i], v_approx[i]);
  // }

  auto createSpline = [](std::vector<float> x, std::vector<float> y) {
    auto spline = Spline1::create({}, 2);
    auto cubic_spline_builder =
        uptr<SplineBuilder>(new CubicSplineBuilder(x, y));
    spline->setSplineType(Spline1::SplineType::CubicSpline);
    spline->setSplineBuilder(std::move(cubic_spline_builder));
    spline->update();
    return spline;
  };

  u_spline = createSpline(s, u_approx);
  v_spline = createSpline(s, v_approx);
  a_spline = createSpline(s, a_approx);
  b_spline = createSpline(s, b_approx);

  cr = Spline1::create({}, 300);
  auto ssb_cr = uptr<SurfaceSplineBuilder>(
      new SurfaceSplineBuilder(_leftSurface, u_spline, v_spline));
  cr->setSplineType(Spline1::SplineType::SurfaceSpline);
  cr->setSplineBuilder(std::move(ssb_cr));
  cr->update();

  cs = Spline1::create({}, 300);
  auto ssb_cs = uptr<SurfaceSplineBuilder>(
      new SurfaceSplineBuilder(_rightSurface, a_spline, b_spline));
  cs->setSplineType(Spline1::SplineType::SurfaceSpline);
  cs->setSplineBuilder(std::move(ssb_cs));
  cs->update();

  auto vao = ENDER::Utils::createParametricSurfaceVAO(
      [&](float u, float v) { return pointOnSurface(u, v); }, 0, 0, 1, 1,
      SURFACE_ROWS, SURFACE_COLS);
  setVertexArray(vao);
  addChildObject(cs);
  addChildObject(cr);
}

sptr<Spline1> FilletSurface::getCrSpline() { return cr; }

sptr<Spline1> FilletSurface::getCsSpline() { return cs; }

glm::vec3 FilletSurface::pointOnSurface(float u, float v) {
  auto crp = cr->getSplinePoint(u);
  auto csp = cs->getSplinePoint(u);

  auto u_cur = u_spline->getSplinePoint(u).z;
  auto v_cur = v_spline->getSplinePoint(u).z;
  auto a_cur = a_spline->getSplinePoint(u).z;
  auto b_cur = b_spline->getSplinePoint(u).z;

  auto mr = _leftSurface->normalOnSurface(u_cur, v_cur);
  auto ms = _rightSurface->normalOnSurface(a_cur, b_cur);

  auto omega =
      (float)(1.0f / glm::sqrt(2.0f) *
              glm::sqrt(1.0f + glm::dot(lR * mr, rR * ms) / (lR * rR)));

  auto c =
      0.5f * (crp + csp -
              (1.0f - omega * omega) * ((lR * mr + rR * ms) / (omega * omega)));

  auto point =
      ((1.0f - v) * (1.0f - v) * crp + 2 * (1.0f - v) * v * omega * c +
       v * v * csp) /
      ((1.0f - v) * (1.0f - v) + 2.0f * (1.0f - v) * v * omega + v * v);

  return point;
}

} // namespace EGEOM
