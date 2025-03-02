#include "Object.hpp"
#include "glm/exponential.hpp"
#include <Surface.hpp>

namespace EGEOM {

Surface::Surface(const std::string &name) : ENDER::Object(name) {};

std::tuple<float, float, float, float> Surface::getUVMinMax() {
  return {u_min, u_max, v_min, v_max};
}

std::vector<std::tuple<float, float>>
Surface::generateAdaptiveGrid(float maxNormalDeviation) {
  std::vector<std::tuple<float, float>> result;

  float u = u_min;
  float v = v_min;

  result.emplace_back(u, v);

  while (u <= u_max) {
    while (v <= v_max) {
      auto [r1, r2] = firstDir(u, v);
      auto [r11, r12, r22] = secondDir(u, v);
      auto normal = normalOnSurface(u, v);

      auto g22 = glm::dot(r2, r2);
      auto b22 = glm::dot(r22, normal);
      auto dv = v_max / 10;
      if (b22 != 0) {
        dv = maxNormalDeviation * glm::sqrt(g22) / b22;
      }
      if (dv < v_max / 10)
        dv = v_max / 10;
      v += dv;
      result.emplace_back(u, v);
    }
    v = 0;
    auto [r1, r2] = firstDir(u, v);
    auto [r11, r12, r22] = secondDir(u, v);
    auto normal = normalOnSurface(u, v);

    auto g11 = glm::dot(r1, r1);
    auto b11 = glm::dot(r11, normal);

    auto du = u_max / 10;
    if (b11 != 0)
      auto du = maxNormalDeviation * glm::sqrt(g11) / b11;
    if (du < u_max / 10)
      du = u_max / 10;
    u += du;
  }
  return result;
}
} // namespace EGEOM
