#include "BufferLayout.hpp"
#include <Line.hpp>

namespace EGEOM {
Line::Line(std::vector<sptr<Point>> points)
    : ENDER::Object("Line"), _points{points} {
  type = ObjectType::Line;

  auto layout = uptr<ENDER::BufferLayout>(
      new ENDER::BufferLayout({{ENDER::LayoutObjectType::Float3}}));
  for (auto point : _points) {
    _rawData.insert(_rawData.end(),
                    {point->getPosition().x, point->getPosition().y,
                     point->getPosition().z});
  }
  auto vbo = std::make_unique<ENDER::VertexBuffer>(std::move(layout));
  vbo->setData(&_rawData[0], _rawData.size() * sizeof(float));
  _vertexArray = std::make_shared<ENDER::VertexArray>();
  _vertexArray->addVBO(std::move(vbo));
}

void Line::addPoint(sptr<Point> point) {
  
  _points.push_back(point);
  _rawData.insert(
      _rawData.end(),
      {point->getPosition().x, point->getPosition().y, point->getPosition().z});

  _vertexArray->setVBOdata(0, &_rawData[0], _rawData.size()*sizeof(float));
}

} // namespace EGEOM
