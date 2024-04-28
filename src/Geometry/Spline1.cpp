#include <Spline1.hpp>

namespace EGEOM {
Spline1::Spline1(const std::vector<sptr<Point>> &points,
                 uint interpolatedPointsCount)
    : ENDER::Object("Spline1") {
  type = ObjectType::Line;
  _splineType = SplineType::LinearInterpolation;
  _splineBuilder = std::make_unique<LinearInterpolationBuilder>(
      points, LinearInterpolationBuilder::ParamMethod::Uniform);
  _interpolatedPointsCount = interpolatedPointsCount;

  auto layout = uptr<ENDER::BufferLayout>(
      new ENDER::BufferLayout({{ENDER::LayoutObjectType::Float3}}));

  auto vbo = std::make_unique<ENDER::VertexBuffer>(std::move(layout));
  vbo->setData(&_rawData[0], _rawData.size() * sizeof(float));
  _vertexArray = std::make_shared<ENDER::VertexArray>();
  _vertexArray->addVBO(std::move(vbo));

  _calculateDrawPoints();
}

void Spline1::_calculateDrawPoints() {
  if (_splineBuilder->points.size() < 2)
    return;

  _interpolatedPoints.clear();
  for (auto i = 0; i < _interpolatedPointsCount; i++) {
    float t = i * 1.f / (_interpolatedPointsCount - 1);
    _interpolatedPoints.push_back(_splineBuilder->getSplinePoint(t));
  }

  _rawData.clear();
  for (auto point : _interpolatedPoints) {
    _rawData.insert(_rawData.end(),
                    {point->getPosition().x, point->getPosition().y,
                     point->getPosition().z});
  }
  _vertexArray->setVBOdata(0, &_rawData[0], _rawData.size() * sizeof(float));
}
void Spline1::setPoints(const std::vector<sptr<Point>> &points) {
  _splineBuilder->points = points;
  update();
}

std::vector<sptr<Point>> Spline1::getInterpolatedPoints() {
  return _interpolatedPoints;
}

void Spline1::setInterpolationPointsCount(uint count) {
  _interpolatedPointsCount = count;
}

void Spline1::update() {
  _splineBuilder->rebuild();
  _calculateDrawPoints();
}

void Spline1::addPoint(sptr<Point> point) {
  _splineBuilder->points.push_back(point);
  update();
}

std::vector<sptr<Point>> Spline1::getPoints() { return _splineBuilder->points; }

sptr<Spline1> Spline1::create(const std::vector<sptr<Point>> &points,
                              uint interpolatedPointsCount) {
  return sptr<Spline1>(new Spline1(points, interpolatedPointsCount));
}

void Spline1::setSplineType(SplineType splineType) { _splineType = splineType; }
void Spline1::setSplineBuilder(uptr<SplineBuilder> splineBuilder) {
  _splineBuilder = std::move(splineBuilder);
}

void Spline1::getPropertiesGUI() {
  if (ImGui::TreeNode("Points")) {

    ImGui::BeginGroup();

    const bool child_is_visible = ImGui::BeginChild("pefe", {0, 200});

    if (child_is_visible) {
      auto i = 0;
      for (auto point : _splineBuilder->points) {
        // if (enable_track && item == track_item)
        // {
        //     ImGui::TextColored(ImVec4(1, 1, 0, 1), "Item %d", item);
        //     ImGui::SetScrollHereY(i * 0.25f); // 0.0f:top,
        //     0.5f:center, 1.0f:bottom
        // }
        // else
        // {
        auto point_name = std::string("Point_") + std::to_string(i);
        ImGui::InputFloat3(point_name.c_str(), glm::value_ptr(point->getPosition()));
        i++;
        // }
      }
    }
    ImGui::EndChild();
    ImGui::EndGroup();

    ImGui::TreePop();
  }
  if (ImGui::TreeNode("Material")) {
    material.drawImguiEdit();
    ImGui::TreePop();
  }
}

} // namespace EGEOM
