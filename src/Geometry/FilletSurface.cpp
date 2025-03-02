#include "Spline1.hpp"
#include "SplineBuilder.hpp"
#include "Surface.hpp"
#include "SurfaceSplineBuilder.hpp"
#include "Utilities.hpp"
#include <FilletSurface.hpp>
#include <algorithm>

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

  auto residual1 = r + lR(s) * mr - sp - rR(s) * ms;

  auto c0dirs = _edge->getEdgeDirs(s, 2);

  auto c0 = c0dirs[0]->getPosition();
  auto c0ds = c0dirs[1]->getPosition();

  auto residual2 =
      glm::dot((c0 - 0.5f * (r - lR(s) * mr + sp + rR(s) * ms)), c0ds);

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
  auto u_min = 0.0;
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
    u = std::max(u, 0.0f);
    v = std::max(v, 0.0f);
    a = std::max(a, 0.0f);
    b = std::max(b, 0.0f);

    u = std::min(u, 1.0f);
    v = std::min(v, 1.0f);
    a = std::min(a, 1.0f);
    b = std::min(b, 1.0f);

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

    u = std::max(u, 0.0f);
    v = std::max(v, 0.0f);
    a = std::max(a, 0.0f);
    b = std::max(b, 0.0f);

    u = std::min(u, 1.0f);
    v = std::min(v, 1.0f);
    a = std::min(a, 1.0f);
    b = std::min(b, 1.0f);
  }
  spdlog::info("iter: {}, tolerance: {}", i, tol);
  // spdlog::info("u_i = {}, v_i = {}, a_i = {}, b_i = {}, s = {}", u, v, a, b,
  // s);

  return {u, v, a, b};
}

glm::vec4 FilletSurface::_findInitialGuess(float s){
  auto adaptivePoints = _edge->getSpline()->generateAdaptivePoints(0.01);
  auto adaptiveLeft = _leftSurface->generateAdaptiveGrid(0.01);
  auto adaptiveRight = _rightSurface->generateAdaptiveGrid(0.01);
  // spdlog::error("ADAP: {}", adaptivePoints.size());
  // spdlog::error("ADAPL: {}", adaptiveLeft.size());
  // spdlog::error("ADAPR: {}", adaptiveRight.size());
  float min_residual = std::numeric_limits<float>::max();
    glm::vec4 best_guess;

    std::vector<glm::vec4> residuals;

    for (auto [u, v] : adaptiveLeft) {
        for (auto [a, b] : adaptiveRight) {
            glm::vec4 residual = _equationSystem(u, v, a, b, s);
            float res_norm = glm::length(residual);
            residuals.emplace_back(glm::vec4(u, v, a, b));

            if (res_norm < min_residual) {
                min_residual = res_norm;
                best_guess = {u, v, a, b};
            }
        }
    }

    // // 4. Ищем смену знака
    // std::pair<float, float> s_interval;
    // std::pair<glm::vec4, glm::vec4> uvab_interval;

    // for (size_t i = 1; i < residuals.size(); ++i) {
    //     auto [s1, uvab1] = residuals[i - 1];
    //     auto [s2, uvab2] = residuals[i];

    //     glm::vec4 r1 = _equationSystem(uvab1.x, uvab1.y, uvab1.z, uvab1.w, s1);
    //     glm::vec4 r2 = _equationSystem(uvab2.x, uvab2.y, uvab2.z, uvab2.w, s2);

    //     if ((glm::length(r1) * glm::length(r2)) < 0) { // Смена знака
    //         s_interval = {s1, s2};
    //         uvab_interval = {uvab1, uvab2};
    //         break;
    //     }
    // }

    // // 5. Бисекция
    // float s_low = s_interval.first, s_high = s_interval.second;
    // glm::vec4 uvab_low = uvab_interval.first, uvab_high = uvab_interval.second;

    // while (fabs(s_high - s_low) > 1e-6) {
    //     float s_mid = (s_low + s_high) / 2.0f;

    //     glm::vec4 uvab_mid = (uvab_low + uvab_high) / 2.0f; // Среднее по всем параметрам

    //     glm::vec4 residual = _equationSystem(uvab_mid.x, uvab_mid.y, uvab_mid.z, uvab_mid.w, s_mid);

    //     if (glm::length(residual) < min_residual) {
    //         min_residual = glm::length(residual);
    //         best_guess = uvab_mid;
    //     }

    //     if ((glm::length(_equationSystem(uvab_low.x, uvab_low.y, uvab_low.z, uvab_low.w, s_low)) *
    //          glm::length(residual)) < 0) {
    //         s_high = s_mid;
    //         uvab_high = uvab_mid;
    //     } else {
    //         s_low = s_mid;
    //         uvab_low = uvab_mid;
    //     }
    // }

    return best_guess;
}


void FilletSurface::update() {
  const float s_step = 1.0 / (SPLINE_APPROX_POINTS - 1);
  std::vector<float> s;

  std::vector<float> u_approx;
  std::vector<float> v_approx;
  std::vector<float> a_approx;
  std::vector<float> b_approx;


  glm::vec4 initialGuess = _findInitialGuess(0);
  // glm::vec4 initialGuess = {0.2, 0.4, 0.3, 0.5};

  spdlog::info("Starting fillet surface creation. Initial guess: {} {} {} {}", initialGuess.x, initialGuess.y, initialGuess.z, initialGuess.w);
  int last_pers = -1;
  auto cur_time = glfwGetTime();
  glm::vec4 prev_approx = {-1, -1, -1, -1};
  for (auto i = 0; i < SPLINE_APPROX_POINTS; i++) {
    auto approx = _newtonMethod(initialGuess.x, initialGuess.y, initialGuess.z,
                                initialGuess.w, i * s_step);

    u_approx.push_back(approx.x);
    v_approx.push_back(approx.y);
    a_approx.push_back(approx.z);
    b_approx.push_back(approx.w);

    auto cur_pers = (int)((float)i / ((float)SPLINE_APPROX_POINTS) * 100.0f);
    if (cur_pers % 10 == 0 && cur_pers != last_pers) {
      auto dt = glfwGetTime() - cur_time;
      spdlog::info("{}% completed, est: {}", cur_pers, dt);
      last_pers = cur_pers;
       cur_time = glfwGetTime();
    }
    s.push_back(i * s_step);
    if(prev_approx != glm::vec4{-1,-1,-1,-1})
      initialGuess = approx + (approx - prev_approx);
    else
      initialGuess = approx;
    prev_approx = approx;
  }
  spdlog::info("Fillet surface creation finished!");

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

  auto omega = (float)(1.0f / glm::sqrt(2.0f) *
                       glm::sqrt(1.0f + glm::dot(lR(u) * mr, rR(u) * ms) /
                                            (lR(u) * rR(u))));

  auto c = 0.5f * (crp + csp -
                   (1.0f - omega * omega) *
                       ((lR(u) * mr + rR(u) * ms) / (omega * omega)));

  auto point =
      ((1.0f - v) * (1.0f - v) * crp + 2 * (1.0f - v) * v * omega * c +
       v * v * csp) /
      ((1.0f - v) * (1.0f - v) + 2.0f * (1.0f - v) * v * omega + v * v);

  return point;
}

} // namespace EGEOM
