#pragma once
#include <Ender.hpp>
#include <Point.hpp>

namespace EGEOM {

class SplineBuilder {
public:
  SplineBuilder(std::vector<sptr<Point>> points) : points(points){};
  std::vector<sptr<Point>> points;

  virtual sptr<Point> getSplinePoint(float t) = 0;
  virtual void rebuild() = 0;
};

class LinearInterpolationBuilder : public SplineBuilder {
public:
  enum class ParamMethod { Uniform, Chordal, Centripetal };

  ParamMethod paramMethod = ParamMethod::Uniform;
  std::vector<float> _t;

  LinearInterpolationBuilder(const std::vector<sptr<Point>> &points,
                             ParamMethod paramMethod)
      : SplineBuilder(points), paramMethod(paramMethod) {
    calculateParameter();
  };

  void rebuild() override { calculateParameter(); };

  sptr<Point> getSplinePoint(float t) override {
    uint j = 0;
    while (t > _t[j + 1])
      j++;

    float h = _t[j + 1] - _t[j];
    float omega = (t - _t[j]) / h;

    glm::vec3 pointPosition;
    pointPosition.x = points[j]->getPosition().x * (1.0f - omega) +
                      points[j + 1]->getPosition().x * omega;
    pointPosition.y = points[j]->getPosition().y * (1.0f - omega) +
                      points[j + 1]->getPosition().y * omega;
    pointPosition.z = points[j]->getPosition().z * (1.0f - omega) +
                      points[j + 1]->getPosition().z * omega;

    return Point::create(pointPosition);
  }

private:
  void calculateParameter() {
    switch (paramMethod) {
    case ParamMethod::Uniform: {
      calculateUniformParameter();
    } break;
    default: {
      spdlog::error("Spline1::_calculateParameter: Not implemented");
    }
    }
  }
  void calculateUniformParameter() {
    _t.clear();
    for (auto i = 0; i < points.size(); i++) {
      _t.push_back(static_cast<float>(i) /
                   (static_cast<float>(points.size()) - 1.0f));
    }
  }
};

class Spline1 : public ENDER::Object {
public:
  enum class ParamMethod { Uniform, Chordal, Centripetal };

  enum class SplineType {
    LinearInterpolation,
    Bezier,
    RationalBezier,
    BSpline,
    NURBS
  };

private:
  int _interpolatedPointsCount;
  std::vector<sptr<Point>> _interpolatedPoints;
  std::vector<float> _rawData;

  SplineType _splineType = SplineType::LinearInterpolation;
  uptr<SplineBuilder> _splineBuilder;
    
  void _calculateDrawPoints();

  Spline1(const std::vector<sptr<Point>> &points,
          uint interpolatedPointsCount);

public:
  static sptr<Spline1> create(const std::vector<sptr<Point>> &points,
                              uint interpolatedPointsCount);

  void addPoint(sptr<Point> point);
  void setPoints(const std::vector<sptr<Point>> &points);
  std::vector<sptr<Point>> getInterpolatedPoints();
  std::vector<sptr<Point>> getPoints();

  void setInterpolationPointsCount(uint count);
  void setSplineType(SplineType splineType);
  void setSplineBuilder(uptr<SplineBuilder> splineBuilder);
    
  void getPropertiesGUI();

  void update();
};
} // namespace EGEOM
