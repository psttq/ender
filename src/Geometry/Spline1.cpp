#include "imgui.h"

#include "ImGuizmo.h"
#include <Spline1.hpp>

namespace EGEOM {
Spline1::Spline1(const std::vector<sptr<Point>> &points,
                 uint interpolatedPointsCount)
    : ENDER::Object("Spline1") {

  label = "Spline";
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
  if (_splineType != SplineType::Parametric &&
      _splineBuilder->getPoints().size() < 2)
    return;

  _interpolatedPoints.clear();
  for (auto i = 0; i < _interpolatedPointsCount; i++) {
    float t = i * u_max / (_interpolatedPointsCount - 1);

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
  _splineBuilder->setPoints(points);
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
  _splineBuilder->addPoint(point);
  update();
}

sptr<Spline1> Spline1::clone() {
  auto splineCopy = Spline1::create({}, _interpolatedPointsCount);
  splineCopy->setSplineType(_splineType);
  splineCopy->setSplineBuilder(_splineBuilder->clone());
  return splineCopy;
}

void Spline1::removePoint(sptr<Point> point) {
  _splineBuilder->removePoint(point);
}

std::vector<sptr<Point>> Spline1::getPoints() {
  return _splineBuilder->getPoints();
}

sptr<Spline1> Spline1::create(const std::vector<sptr<Point>> &points,
                              uint interpolatedPointsCount) {
  return sptr<Spline1>(new Spline1(points, interpolatedPointsCount));
}

void Spline1::setSplineType(SplineType splineType) { _splineType = splineType; }

void Spline1::setSplineBuilder(uptr<SplineBuilder> splineBuilder) {
  _splineBuilder = std::move(splineBuilder);
}

void Spline1::getPropertiesGUI(bool scrollToPoint) {
  std::vector<const char *> items = {"Linear Interpolation",
                                     "Bezier",
                                     "Rational Bezier",
                                     "BSpline",
                                     "NURBS",
                                     "Parametric"};
  int currentItem = static_cast<int>(_splineType);

  if (ImGui::Combo("Spline Type", &currentItem, &items[0], items.size())) {
    auto splineType = static_cast<SplineType>(currentItem);
    if (_splineType != splineType) {
      _splineType = splineType;
      auto points = _splineBuilder->getPoints();
      switch (_splineType) {
      case SplineType::Bezier: {
        auto bezierBuilder =
            std::make_unique<BezierBuilder>(points, points.size() - 1);
        setSplineBuilder(std::move(bezierBuilder));
      } break;
      case SplineType::LinearInterpolation: {
        auto linearBuilder = std::make_unique<LinearInterpolationBuilder>(
            points, LinearInterpolationBuilder::ParamMethod::Uniform);
        setSplineBuilder(std::move(linearBuilder));
      } break;
      case SplineType::RationalBezier: {
        std::vector<float> weighs = {};
        auto rationalBezierBuilder = std::make_unique<RationalBezierBuilder>(
            points, points.size() - 1, weighs);
        setSplineBuilder(std::move(rationalBezierBuilder));
      } break;
      case SplineType::BSpline: {
        std::vector<float> knotVector = {};
        auto bsplineBuilder =
            std::make_unique<BSplineBuilder>(points, 1, knotVector);
        setSplineBuilder(std::move(bsplineBuilder));
      } break;
      case SplineType::NURBS: {
        std::vector<float> knotVector = {};
        std::vector<float> weights = {};
        auto nurbsBuilder = std::make_unique<RationalBSplineBuilder>(
            points, 1, knotVector, weights);
        setSplineBuilder(std::move(nurbsBuilder));
      } break;
      }
      update();
    }
  }

  ImGui::Checkbox("isDirectionInversed", &_isDirectionInversed);

  if (ImGui::TreeNode("Points")) {
    ImGui::BeginGroup();
    const bool child_is_visible = ImGui::BeginChild("pefe", {0, 200});

    if (child_is_visible) {
      auto i = 0;
      for (auto point : _splineBuilder->getPoints()) {
        auto point_name = std::string("Point_") + std::to_string(i);
        if (point->selected()) {
          ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
          ImGui::InputFloat3(point_name.c_str(),
                             glm::value_ptr(point->getPosition()));
          ImGui::PopStyleColor();
          if (scrollToPoint)
            ImGui::SetScrollHereY(0.25f);
        } else
          ImGui::InputFloat3(point_name.c_str(),
                             glm::value_ptr(point->getPosition()));
        i++;
        // }
      }
    }
    ImGui::EndChild();
    ImGui::EndGroup();

    ImGui::TreePop();
  }
  if (ImGui::TreeNode(items[currentItem])) {
    if (_splineBuilder->drawPropertiesGui())
      update();
    ImGui::TreePop();
  }
  if (ImGui::TreeNode("Draw Points")) {
    ImGui::BeginGroup();
    const bool child_is_visible = ImGui::BeginChild("pefe", {0, 200});

    if (child_is_visible) {
      auto i = 0;
      for (auto point : _interpolatedPoints) {
        auto point_name = std::string("Point_") + std::to_string(i);
        ImGui::InputFloat3(point_name.c_str(),
                           glm::value_ptr(point->getPosition()), "%.3f",
                           ImGuiInputTextFlags_ReadOnly);
        i++;
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

Spline1::SplineType Spline1::getSplineType() const { return _splineType; }

glm::vec3 Spline1::getSplinePoint(float u) {
  auto point = _splineBuilder->getSplinePoint(u);
  return point->getPosition();
}

std::vector<sptr<Point>> Spline1::getSplineDirs(float u, int dirsCount) {
  return _splineBuilder->getSplineDerivatives(u, dirsCount);
}

void Spline1::drawGizmo() {
  auto points = _splineBuilder->getPoints();
  if (points.size() < 2)
    return;
  for (auto p1i = points.begin(), end = points.end(); p1i + 1 != end; ++p1i) {
    auto p2i = p1i + 1;
    if (p2i == end)
      break;

    auto p1 = p1i->get()->getPosition();
    auto p2 = p2i->get()->getPosition();
    if (_isDirectionInversed) {
      auto tmp = p1;
      p1 = p2;
      p2 = tmp;
    }
    ImGuizmo::DrawArrow({p1.x, p1.y, p1.z, 0}, {p2.x, p2.y, p2.z, 0},
                        0x44FF44FF);
  }
}

} // namespace EGEOM
