#include "SplineBuilder.hpp"
#include "imgui.h"

#include <Spline1.hpp>
#include <cmath>

namespace EGEOM
{
  Spline1::Spline1(const std::vector<sptr<Point>> &points,
                   uint interpolatedPointsCount)
      : ENDER::Object("Spline1")
  {

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

  void Spline1::_calculateDrawPoints()
  {
    if (_splineType != SplineType::Parametric &&
        _splineType != SplineType::SurfaceSpline &&
        _splineBuilder->getPoints().size() < 2)
      return;

    _interpolatedPoints.clear();
    for (auto i = 0; i < _interpolatedPointsCount; i++)
    {
      float t = u_min + i * (u_max - u_min) / (_interpolatedPointsCount - 1);

      _interpolatedPoints.push_back(_splineBuilder->getSplinePoint(t));
    }

    _rawData.clear();
    for (auto point : _interpolatedPoints)
    {
      _rawData.insert(_rawData.end(),
                      {point->getPosition().x, point->getPosition().y,
                       point->getPosition().z});
    }
    _vertexArray->setVBOdata(0, &_rawData[0], _rawData.size() * sizeof(float));
  }

  void Spline1::setPoints(const std::vector<sptr<Point>> &points)
  {
    _splineBuilder->setPoints(points);
    update();
  }

  std::vector<sptr<Point>> Spline1::getInterpolatedPoints()
  {
    return _interpolatedPoints;
  }

  void Spline1::setInterpolationPointsCount(uint count)
  {
    _interpolatedPointsCount = count;
  }

  glm::vec2 projectToLocal(const glm::vec3 &point, const glm::vec3 &origin, const glm::vec3 &u, const glm::vec3 &v)
  {
    glm::vec3 relative = point - origin;
    double x = glm::dot(relative, u);
    double y = glm::dot(relative, v);
    return {x, y};
  }

  std::tuple<float, float> Spline1::intersect(sptr<Spline1> spline,
                                              glm::vec2 t_initial,
                                              float tolerance, int maxIter)
  {
      auto t = t_initial;
      for (int iter = 0; iter < maxIter; ++iter) {
           // Вычисляем текущие точки на кривых
           glm::vec3 p1 = getSplinePoint(t.x);
           glm::vec3 p2 = spline->getSplinePoint(t.y);

           // Вычисляем разницу в 3D
           glm::vec3 delta = p1 - p2;

           // Проекция на плоскость
           glm::vec3 origin = 0.5f * (p1 + p2);
           glm::vec3 i1 = getSplineDirs(t.x,2)[1]->getPosition();
           glm::vec3 i2 = spline->getSplineDirs(t.y,2)[1]->getPosition();

           glm::vec3 n = glm::cross(i1, i2); // Нормаль плоскости
           glm::mat3 projectionMatrix(1.0f);
           projectionMatrix[0] = i1;
           projectionMatrix[1] = i2;
           projectionMatrix[2] = glm::normalize(n);

           glm::vec2 delta2D = glm::vec2(glm::transpose(projectionMatrix) * delta); // Проекция разности

           // Якобиан (матрица производных)
           glm::mat2 J;
           J[0][0] = glm::dot(i1, i1);
           J[0][1] = -glm::dot(i2, i1);
           J[1][0] = glm::dot(i1, i2);
           J[1][1] = -glm::dot(i2, i2);

           // Решение линейной системы J * dt = -delta2D
           glm::vec2 dt = glm::inverse(J) * (-delta2D);

           // Обновляем параметры
           t += dt;

           // Проверяем условие выхода
           if (glm::length(dt) < tolerance) {
               return {t.x,t.y};
           }

           if(std::isnan(t.x) || std::isnan(t.y)){
               spdlog::error("Spline::intersect: Get nan");
               return {-100, -100};
           }
           spdlog::error("iter: {}, t: {} {}, dt: {} ", iter, t.x,t.y, glm::length(dt));
       }

    return {-100, -100}; // Не удалось найти пересечение
  }

  void Spline1::update()
  {
    _splineBuilder->rebuild();
    _calculateDrawPoints();
  }

  void Spline1::addPoint(sptr<Point> point)
  {
    _splineBuilder->addPoint(point);
    update();
  }

  sptr<Spline1> Spline1::clone()
  {
    auto splineCopy = Spline1::create({}, _interpolatedPointsCount);
    splineCopy->setSplineType(_splineType);
    splineCopy->setSplineBuilder(_splineBuilder->clone());
    return splineCopy;
  }

  void Spline1::removePoint(sptr<Point> point)
  {
    _splineBuilder->removePoint(point);
  }

  std::vector<sptr<Point>> Spline1::getPoints()
  {
    return _splineBuilder->getPoints();
  }

  sptr<Spline1> Spline1::create(const std::vector<sptr<Point>> &points,
                                uint interpolatedPointsCount)
  {
    return sptr<Spline1>(new Spline1(points, interpolatedPointsCount));
  }

  void Spline1::setSplineType(SplineType splineType) { _splineType = splineType; }

  void Spline1::setSplineBuilder(uptr<SplineBuilder> splineBuilder)
  {
    _splineBuilder = std::move(splineBuilder);
  }

  void Spline1::getPropertiesGUI(bool scrollToPoint)
  {
    std::vector<const char *> items = {
        "Linear Interpolation", "Bezier", "Rational Bezier", "BSpline", "NURBS",
        "Parametric", "Cubic"};
    int currentItem = static_cast<int>(_splineType);

    if (ImGui::Combo("Spline Type", &currentItem, &items[0], items.size()))
    {
      auto splineType = static_cast<SplineType>(currentItem);
      if (_splineType != splineType)
      {
        _splineType = splineType;
        auto points = _splineBuilder->getPoints();
        switch (_splineType)
        {
        case SplineType::Bezier:
        {
          auto bezierBuilder =
              std::make_unique<BezierBuilder>(points, points.size() - 1);
          setSplineBuilder(std::move(bezierBuilder));
        }
        break;
        case SplineType::LinearInterpolation:
        {
          auto linearBuilder = std::make_unique<LinearInterpolationBuilder>(
              points, LinearInterpolationBuilder::ParamMethod::Uniform);
          setSplineBuilder(std::move(linearBuilder));
        }
        break;
        case SplineType::RationalBezier:
        {
          std::vector<float> weighs = {};
          auto rationalBezierBuilder = std::make_unique<RationalBezierBuilder>(
              points, points.size() - 1, weighs);
          setSplineBuilder(std::move(rationalBezierBuilder));
        }
        break;
        case SplineType::BSpline:
        {
          std::vector<float> knotVector = {};
          auto bsplineBuilder =
              std::make_unique<BSplineBuilder>(points, 1, knotVector);
          setSplineBuilder(std::move(bsplineBuilder));
        }
        break;
        case SplineType::NURBS:
        {
          std::vector<float> knotVector = {};
          std::vector<float> weights = {};
          auto nurbsBuilder = std::make_unique<RationalBSplineBuilder>(
              points, 1, knotVector, weights);
          setSplineBuilder(std::move(nurbsBuilder));
        }
        break;
        case SplineType::CubicSpline:
        {
          auto cubicBuilder = std::make_unique<CubicSplineBuilder>(points);
          setSplineBuilder(std::move(cubicBuilder));
        }
        break;
        }
        update();
      }
    }

    ImGui::Checkbox("isDirectionInversed", &_isDirectionInversed);

    if (ImGui::TreeNode("Points"))
    {
      ImGui::BeginGroup();
      const bool child_is_visible = ImGui::BeginChild("pefe", {0, 200});

      if (child_is_visible)
      {
        auto i = 0;
        for (auto point : _splineBuilder->getPoints())
        {
          auto point_name = std::string("Point_") + std::to_string(i);
          if (point->selected())
          {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::InputFloat3(point_name.c_str(),
                               glm::value_ptr(point->getPosition()));
            ImGui::PopStyleColor();
            if (scrollToPoint)
              ImGui::SetScrollHereY(0.25f);
          }
          else
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
    if (ImGui::TreeNode(items[currentItem]))
    {
      if (_splineBuilder->drawPropertiesGui())
        update();
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Draw Points"))
    {
      ImGui::BeginGroup();
      const bool child_is_visible = ImGui::BeginChild("pefe", {0, 200});

      if (child_is_visible)
      {
        auto i = 0;
        for (auto point : _interpolatedPoints)
        {
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
    if (ImGui::TreeNode("Material"))
    {
      material.drawImguiEdit();
      ImGui::TreePop();
    }
  }

  Spline1::SplineType Spline1::getSplineType() const { return _splineType; }

  glm::vec3 Spline1::getSplinePoint(float u)
  {
    auto point = _splineBuilder->getSplinePoint(u);
    return point->getPosition();
  }

  std::vector<sptr<Point>> Spline1::getSplineDirs(float u, int dirsCount)
  {
    return _splineBuilder->getSplineDerivativesSave(u, dirsCount);
  }

} // namespace EGEOM
