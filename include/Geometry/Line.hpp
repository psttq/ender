#pragma once
#include "Object.hpp"
#include <Point.hpp>
#include <Ender.hpp>

namespace EGEOM {
class Line : public ENDER::Object {
  std::vector<sptr<Point>> _points;
  std::vector<float> _rawData;

public:
  Line(std::vector<sptr<Point>> points);

  void addPoint(sptr<Point> point);

};
} // namespace EGEOM
