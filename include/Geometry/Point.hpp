#pragma once
#include <Ender.hpp>

namespace EGEOM {
class Point : public ENDER::Object {
public:
  Point(const glm::vec3 &position);

  static sptr<Point> create(const glm::vec3 &position);

  sptr<Point> copy();

  Point operator+(const Point &other) {
    auto thisPos = getPosition();
    auto otherPos = other.getPosition();
    return {{thisPos.x + otherPos.x, thisPos.y + otherPos.y,
             thisPos.z + otherPos.z}};
  }
  Point operator-(const Point &other) {
    auto thisPos = getPosition();
    auto otherPos = other.getPosition();
    return {{thisPos.x - otherPos.x, thisPos.y - otherPos.y,
             thisPos.z - otherPos.z}};
  }
  Point &operator+=(const Point &other) {
    auto thisPos = getPosition();
    auto otherPos = other.getPosition();
    _position = {thisPos.x + otherPos.x, thisPos.y + otherPos.y,
                 thisPos.z + otherPos.z};
    return *this;
  }

  friend Point operator*(float x, const Point &point) {
    auto pos = point.getPosition();
    return {{pos.x * x, pos.y * x, pos.z * x}};
  }
};
} // namespace EGEOM
