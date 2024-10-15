#include "Object.hpp"
#include <Surface.hpp>

namespace EGEOM {

Surface::Surface(const std::string &name) : ENDER::Object(name) {};

std::tuple<float, float, float, float> Surface::getUVMinMax() {
  return {u_min, u_max, v_min, v_max};
}
} // namespace EGEOM
