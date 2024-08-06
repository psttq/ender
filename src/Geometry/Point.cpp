#include <Point.hpp>

namespace EGEOM {
Point::Point(const glm::vec3 &position)
    : ENDER::Object("Point", ENDER::Renderer::getCubeVAO()) {
  _scale = {0.1, 0.1, 0.1};
  _position = position;
}

sptr<Point> Point::copy() { return sptr<Point>(new Point(*this)); }

sptr<Point> Point::clone() { return sptr<Point>(new Point(_position));}

sptr<Point> Point::create(const glm::vec3 &position) {
  return sptr<Point>(new Point(position));
}

} // namespace EGEOM
