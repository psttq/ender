#pragma once

#include <Ender.hpp>
#include <Point.hpp>
#include <SplineBuilder.hpp>

namespace EGEOM {

class Spline1 : public ENDER::Object {
public:
  enum class ParamMethod { Uniform, Chordal, Centripetal };

  enum class SplineType : int {
    LinearInterpolation,
    Bezier,
    RationalBezier,
    BSpline,
    NURBS,
    Parametric
  };

  float u_max = 1.0f;


private:
  int _interpolatedPointsCount;
  std::vector<sptr<Point>> _interpolatedPoints;
  std::vector<float> _rawData;

  SplineType _splineType = SplineType::LinearInterpolation;
  uptr<SplineBuilder> _splineBuilder;

  bool _isDirectionInversed = false;

  void _calculateDrawPoints();

  Spline1(const std::vector<sptr<Point>> &points, uint interpolatedPointsCount);

public:
  static sptr<Spline1> create(const std::vector<sptr<Point>> &points,
                              uint interpolatedPointsCount);

  void addPoint(sptr<Point> point);
  void removePoint(sptr<Point> point);

  void setPoints(const std::vector<sptr<Point>> &points);

  std::vector<sptr<Point>> getInterpolatedPoints();

  std::vector<sptr<Point>> getPoints();

  glm::vec3 getSplinePoint(float u);

  std::vector<sptr<Point>> getSplineDirs(float u, int dirsCount);

  void setInterpolationPointsCount(uint count);

  void setSplineType(SplineType splineType);

  SplineType getSplineType() const;

  void setSplineBuilder(uptr<SplineBuilder> splineBuilder);

  template <typename T> T &getSplineBuilder() {
    return *reinterpret_cast<T *>(_splineBuilder.get());
  }

  void getPropertiesGUI(bool scrollToPoint);

  void update();


  sptr<Spline1> clone();
};
} // namespace EGEOM
