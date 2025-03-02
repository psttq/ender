#pragma once

#include <Point.hpp>
#include <vector>
namespace EGEOM {

class SplineBuilder {
public:
  bool shouldHandleParameterOverflow = true;
protected:
  std::vector<sptr<Point>> points;
  bool isDirected = false;

public:
  SplineBuilder(const std::vector<sptr<Point>> &points) : points(points) {};

  virtual ~SplineBuilder() = default;

  void addPoint(sptr<Point> point);
  void removePoint(sptr<Point> point);
  void setPoints(std::vector<sptr<Point>> new_points);

  std::vector<sptr<Point>> getPoints();

  virtual sptr<Point> _getSplinePoint(float t) = 0;

  virtual sptr<Point> getSplinePoint(float t) {
    if(shouldHandleParameterOverflow){
      if (t > 1.0f) {
        return aboveSplinePoint(t);
      }
      if (t < 0.0f) {
        return belowSplinePoint(t);
      }
    }
    return _getSplinePoint(t);
  }

  sptr<Point> aboveSplinePoint(float t) {
    auto point1 = _getSplinePoint(1.0f);
    auto glm_point = point1->getPosition();
    auto dir = getSplineDerivatives(1.0, 3)[1]->getPosition();
    glm::vec3 point = glm_point + (t - 1.0f) * dir;
    return Point::create(point);
  }

  sptr<Point> belowSplinePoint(float t) {
    auto point1 = _getSplinePoint(0.0f);
    auto glm_point = point1->getPosition();
    auto dir = getSplineDerivatives(0.0, 3)[1]->getPosition();
    glm::vec3 point = glm_point + (t - 0.0f) * dir;
    return Point::create(point);
  }

  virtual std::vector<sptr<Point>> getSplineDerivatives(float t,
                                                        int dirsCount) {
    return {};
  }

  std::vector<sptr<Point>> getSplineDerivativesSave(float t, int dirsCount) {
    if (t > 1.0)
      return getSplineDerivatives(1.0, dirsCount);
    if (t < 0.0)
      return getSplineDerivatives(0.0, dirsCount);
    return getSplineDerivatives(t, dirsCount);
  }

  virtual void rebuild() = 0;

  virtual bool drawPropertiesGui() = 0;

  virtual uptr<SplineBuilder> clone() = 0;
};

/////////////////////////////////////
/// ParametricBuilder
/////////////////////////////////////

class ParametricBuilder : public SplineBuilder {
public:
  using ParametricFunction = std::function<glm::vec3(float)>;

private:
  ParametricFunction _paramFunc;

public:
  ParametricBuilder(ParametricFunction paramFunc);

  sptr<Point> _getSplinePoint(float t) override;

  void rebuild() override;

  bool drawPropertiesGui() override;

  uptr<SplineBuilder> clone() override;
};

/////////////////////////////////////
/// LinearInterpolationBuilder
/////////////////////////////////////

class LinearInterpolationBuilder : public SplineBuilder {
public:
  enum class ParamMethod : int { Uniform, Chordal, Centripetal };

  ParamMethod paramMethod = ParamMethod::Uniform;
  std::vector<float> _t;

  LinearInterpolationBuilder(const std::vector<sptr<Point>> &points,
                             ParamMethod paramMethod);
  void rebuild() override;

  sptr<Point> _getSplinePoint(float t) override;

  bool drawPropertiesGui() override;
  std::vector<sptr<Point>> getSplineDerivatives(float t, int dirsCount) override;

  uptr<SplineBuilder> clone() override;

private:
  void calculateParameter();
  void calculateUniformParameter();
  sptr<Point> _getSplineFirstDerivative(float t);
  sptr<Point> _getSplineSecondDerivative(float t);

};

/////////////////////////////////////
/// CubicSplineBuilder
/////////////////////////////////////

class CubicSplineBuilder : public SplineBuilder {
  struct SplineSegment {
    float a, b, c, d; // Коэффициенты кубического полинома
  };
  size_t n;
  std::vector<float> x, y;
  std::vector<SplineSegment> coefficients;

  float x_min;
  float x_max;

  sptr<Point> _getSplineFirstDerivative(float t);
  sptr<Point> _getSplineSecondDerivative(float t);

public:
  CubicSplineBuilder(const std::vector<sptr<Point>> &points);
  CubicSplineBuilder(const std::vector<float> &x, const std::vector<float> &y);

  void rebuild() override;

  sptr<Point> _getSplinePoint(float t) override;

  std::vector<sptr<Point>> getSplineDerivatives(float t,
                                                int dirsCount) override;


  bool drawPropertiesGui() override;

  uptr<SplineBuilder> clone() override;
};

/////////////////////////////////////
/// BezierBuilder
/////////////////////////////////////

class BezierBuilder : public SplineBuilder {
  std::vector<float> _allBernstein(float u);

  sptr<Point> _deCasteljau(float u);

  sptr<Point> pointWithAllBernstein(float u);

  std::vector<glm::vec3> _glmPoints;

public:
  int bezierPower = 3;
  bool useDeCasteljau = true;

  BezierBuilder(const std::vector<sptr<Point>> &points, int bezierPower);

  sptr<Point> _getSplinePoint(float t) override;

  void rebuild() override;

  bool drawPropertiesGui() override;

  uptr<SplineBuilder> clone() override;
};

/////////////////////////////////////
/// RationalBezierBuilder
/////////////////////////////////////

class RationalBezierBuilder : public SplineBuilder {
private:
  std::vector<glm::vec3> _glmPoints;

  std::vector<float> _allBernstein(float u);
  sptr<Point> _deCasteljau(float u);

public:
  std::vector<float> w;
  int bezierPower = 3;

  RationalBezierBuilder(const std::vector<sptr<Point>> &points, int bezierPower,
                        const std::vector<float> &weights);

  sptr<Point> _getSplinePoint(float t) override;
  void rebuild() override;
  bool drawPropertiesGui() override;
  uptr<SplineBuilder> clone() override;
};

/////////////////////////////////////
/// BSplineBuilder
/////////////////////////////////////

class BSplineBuilder : public SplineBuilder {
  void _checkAndSetDefault();

public:
  int bSplinePower = 0;
  std::vector<float> knotVector = {};

  BSplineBuilder(const std::vector<sptr<Point>> &points, int bSplinePower,
                 const std::vector<float> &knotVector);

  sptr<Point> _getSplinePoint(float t) override;
  std::vector<sptr<Point>> getSplineDerivatives(float t,
                                                int dirsCount) override;
  void rebuild() override;
  bool drawPropertiesGui() override;
  uptr<SplineBuilder> clone() override;
};

/////////////////////////////////////
/// RationalBSplineBuilder
/////////////////////////////////////

class RationalBSplineBuilder : public SplineBuilder {

  void _checkAndSetDefault();
  std::vector<glm::vec4> _glmPoints;

public:
  int bSplinePower = 0;
  std::vector<float> knotVector = {};
  std::vector<float> weights = {};

  RationalBSplineBuilder(const std::vector<sptr<Point>> &points,
                         int bSplinePower, const std::vector<float> &knotVector,
                         const std::vector<float> &weights);
  sptr<Point> _getSplinePoint(float t) override;

  std::vector<sptr<Point>> getSplineDerivatives(float t,
                                                int dirsCount) override;
  void rebuild() override;
  bool drawPropertiesGui() override;
  uptr<SplineBuilder> clone() override;
};
} // namespace EGEOM
