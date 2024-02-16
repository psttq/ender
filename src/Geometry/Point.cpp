#include <Point.hpp>

namespace EGEOM {
Point::Point(const glm::vec3 &position)
    : ENDER::Object("Point", ENDER::Renderer::getCircleVAO()) {
  _position = position;
}

sptr<Point> Point::create(const glm::vec3 &position) {
  return sptr<Point>(new Point(position));
}
} // namespace EGEOM
