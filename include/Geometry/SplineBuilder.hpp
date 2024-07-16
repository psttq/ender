#pragma once

#include <Point.hpp>
#include <vector>
namespace EGEOM
{

  class SplineBuilder
  {
  public:
    SplineBuilder(const std::vector<sptr<Point>> &points) : points(points){};
    std::vector<sptr<Point>> points;

    virtual ~SplineBuilder() = default;

    virtual sptr<Point> getSplinePoint(float t) = 0;

    virtual std::vector<sptr<Point>> getSplineDerivatives(float t,
                                                          int dirsCount)
    {
      return {};
    }

    virtual void rebuild() = 0;

    virtual bool drawPropertiesGui() = 0;
  };

  /////////////////////////////////////
  /// ParametricBuilder
  /////////////////////////////////////

  class ParametricBuilder : public SplineBuilder
  {
  public:
    using ParametricFunction = std::function<glm::vec3(float)>;

  private:
    ParametricFunction _paramFunc;

  public:
    ParametricBuilder(ParametricFunction paramFunc);

    sptr<Point> getSplinePoint(float t) override;

    void rebuild() override;

    bool drawPropertiesGui() override;
  };

  /////////////////////////////////////
  /// LinearInterpolationBuilder
  /////////////////////////////////////

  class LinearInterpolationBuilder : public SplineBuilder
  {
  public:
    enum class ParamMethod : int
    {
      Uniform,
      Chordal,
      Centripetal
    };

    ParamMethod paramMethod = ParamMethod::Uniform;
    std::vector<float> _t;

    LinearInterpolationBuilder(const std::vector<sptr<Point>> &points,
                               ParamMethod paramMethod);
    void rebuild() override;

    sptr<Point> getSplinePoint(float t) override;

    bool drawPropertiesGui() override;

  private:
    void calculateParameter();
    void calculateUniformParameter();
  };

  /////////////////////////////////////
  /// BezierBuilder
  /////////////////////////////////////

  class BezierBuilder : public SplineBuilder
  {
    std::vector<float> _allBernstein(float u);

    sptr<Point> _deCasteljau(float u);

    sptr<Point> pointWithAllBernstein(float u);

    std::vector<glm::vec3> _glmPoints;

  public:
    int bezierPower = 3;
    bool useDeCasteljau = true;

    BezierBuilder(const std::vector<sptr<Point>> &points, int bezierPower);

    sptr<Point> getSplinePoint(float t) override;

    void rebuild() override;

    bool drawPropertiesGui() override;
  };

  /////////////////////////////////////
  /// RationalBezierBuilder
  /////////////////////////////////////

  class RationalBezierBuilder : public SplineBuilder
  {
  private:
    std::vector<glm::vec3> _glmPoints;

    std::vector<float> _allBernstein(float u);
    sptr<Point> _deCasteljau(float u);

  public:
    std::vector<float> w;
    int bezierPower = 3;

    RationalBezierBuilder(const std::vector<sptr<Point>> &points, int bezierPower,
                          const std::vector<float> &weights);

    sptr<Point> getSplinePoint(float t) override;
    void rebuild() override;
    bool drawPropertiesGui() override;
  };

  /////////////////////////////////////
  /// BSplineBuilder
  /////////////////////////////////////

  class BSplineBuilder : public SplineBuilder
  {
    void _checkAndSetDefault();

  public:
    int bSplinePower = 0;
    std::vector<float> knotVector = {};

    BSplineBuilder(const std::vector<sptr<Point>> &points, int bSplinePower,
                   const std::vector<float> &knotVector);

    sptr<Point> getSplinePoint(float t) override;
    std::vector<sptr<Point>> getSplineDerivatives(float t,
                                                  int dirsCount) override;
    void rebuild() override;
    bool drawPropertiesGui() override;
  };

  /////////////////////////////////////
  /// RationalBSplineBuilder
  /////////////////////////////////////

  class RationalBSplineBuilder : public SplineBuilder
  {

    void _checkAndSetDefault();
    std::vector<glm::vec4> _glmPoints;

  public:
    int bSplinePower = 0;
    std::vector<float> knotVector = {};
    std::vector<float> weights = {};

    RationalBSplineBuilder(const std::vector<sptr<Point>> &points,
                           int bSplinePower, const std::vector<float> &knotVector,
                           const std::vector<float> &weights);
    sptr<Point> getSplinePoint(float t) override;

    std::vector<sptr<Point>> getSplineDerivatives(float t,
                                                  int dirsCount) override;
    void rebuild() override;
    bool drawPropertiesGui() override;
  };
} // namespace EGEOM
