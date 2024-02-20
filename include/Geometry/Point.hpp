#pragma once
#include <Ender.hpp>

namespace EGEOM
{
  class Point : public ENDER::Object
  {
  public:
    Point(const glm::vec3 &position);

    static sptr<Point> create(const glm::vec3 &position);
  };
} // namespace EGEOM
