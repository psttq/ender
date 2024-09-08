#include "Point.hpp"
#include "Utils.hpp"
#include <SplineBuilder.hpp>
#include <ranges>

namespace EGEOM {

void SplineBuilder::addPoint(sptr<Point> point){
    points.push_back(point);
}

void SplineBuilder::removePoint(sptr<Point> point){
    auto iter = std::remove(points.begin(),
                            points.end(), point);
    points.erase(iter, points.end());
}

std::vector<sptr<Point>> SplineBuilder::getPoints(){
    return points;
}

void SplineBuilder::setPoints(std::vector<sptr<Point>> new_points){
    points = new_points;
}

/////////////////////////////////////
/// ParametricBuilder
/////////////////////////////////////

ParametricBuilder::ParametricBuilder(ParametricFunction paramFunc)
    : SplineBuilder({}), _paramFunc(paramFunc) {}

sptr<Point> ParametricBuilder::getSplinePoint(float t) {
  return Point::create(_paramFunc(t));
}

void ParametricBuilder::rebuild() {}

bool ParametricBuilder::drawPropertiesGui() {
  ImGui::Text("Parametric Builder");
  return false;
}

uptr<SplineBuilder> ParametricBuilder::clone() {
  return uptr<ParametricBuilder>(new ParametricBuilder(_paramFunc));
}

/////////////////////////////////////
/// LinearInterpolationBuilder
/////////////////////////////////////

LinearInterpolationBuilder::LinearInterpolationBuilder(
    const std::vector<sptr<Point>> &points, ParamMethod paramMethod)
    : SplineBuilder(points), paramMethod(paramMethod) {
  calculateParameter();
};

uptr<SplineBuilder> LinearInterpolationBuilder::clone() {
  std::vector<sptr<Point>> pointsCopy;
  for (auto point : points) {
    pointsCopy.push_back(point->clone());
  }
  return uptr<LinearInterpolationBuilder>(
      new LinearInterpolationBuilder(pointsCopy, paramMethod));
}

void LinearInterpolationBuilder::rebuild() { calculateParameter(); };

sptr<Point> LinearInterpolationBuilder::getSplinePoint(float t) {
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

bool LinearInterpolationBuilder::drawPropertiesGui() {
  std::vector<const char *> items = {
      "Uniform",
  };
  int currentItem = static_cast<int>(paramMethod);

  if (ImGui::Combo("Parameter Calculation Method", &currentItem, &items[0],
                   items.size())) {
  }

  if (ImGui::TreeNode("Parameter")) {
    ImGui::BeginGroup();
    const bool child_is_visible = ImGui::BeginChild("pefe", {0, 200});

    if (child_is_visible) {
      auto i = 0;
      for (auto t : _t) {
        auto point_name = std::string("Point_") + std::to_string(i) + "_t";
        ImGui::InputFloat(point_name.c_str(), &t, 0, 0.0f, "%.3f",
                          ImGuiInputTextFlags_ReadOnly);
        i++;
      }
    }
    ImGui::EndChild();
    ImGui::EndGroup();

    ImGui::TreePop();
  }
  return false;
}

void LinearInterpolationBuilder::calculateParameter() {
  switch (paramMethod) {
  case ParamMethod::Uniform: {
    calculateUniformParameter();
  } break;
  default: {
    spdlog::error("Spline1::_calculateParameter: Not implemented");
  }
  }
}

void LinearInterpolationBuilder::calculateUniformParameter() {
  _t.clear();
  for (auto i = 0; i < points.size(); i++) {
    _t.push_back(static_cast<float>(i) /
                 (static_cast<float>(points.size()) - 1.0f));
  }
}

/////////////////////////////////////
/// BezierBuilder
/////////////////////////////////////

std::vector<float> BezierBuilder::_allBernstein(float u) {
  std::vector<float> B(bezierPower + 1, 0);
  B[0] = 1.0f;
  float u1 = 1.0f - u;
  for (int j = 1; j <= bezierPower; j++) {
    float saved = 0.0f;
    for (int k = 0; k < j; k++) {
      float temp = B[k];
      B[k] = saved + u1 * temp;
      saved = u * temp;
    }
    B[j] = saved;
  }
  return B;
}

sptr<Point> BezierBuilder::_deCasteljau(float u) {
  std::vector<glm::vec3> _glmPointsCopy;
  std::copy(_glmPoints.begin(), _glmPoints.end(),
            std::back_inserter(_glmPointsCopy));
  for (int k = 1; k <= bezierPower; k++) {
    for (int i = 0; i < bezierPower - k + 1; i++) {
      _glmPointsCopy[i] =
          (1.0f - u) * _glmPointsCopy[i] + u * _glmPointsCopy[i + 1];
    }
  }
  return Point::create(_glmPointsCopy[0]);
}

sptr<Point> BezierBuilder::pointWithAllBernstein(float u) {
  auto B = _allBernstein(u);
  auto C = Point::create({0, 0, 0});
  for (int k = 0; k <= bezierPower; k++) {
    *C = *C + B[k] * (*(points[k]));
  }
  return C;
}

std::vector<glm::vec3> _glmPoints;

BezierBuilder::BezierBuilder(const std::vector<sptr<Point>> &points,
                             int bezierPower)
    : SplineBuilder(points), bezierPower(bezierPower) {}

uptr<SplineBuilder> BezierBuilder::clone() {
  std::vector<sptr<Point>> pointsCopy;
  for (auto point : points) {
    pointsCopy.push_back(point->clone());
  }
  return uptr<BezierBuilder>(new BezierBuilder(pointsCopy, bezierPower));
}

sptr<Point> BezierBuilder::getSplinePoint(float t) {
  if (useDeCasteljau) {
    return _deCasteljau(t);
  }
  return pointWithAllBernstein(t);
}

void BezierBuilder::rebuild() {
  bezierPower = points.size() - 1;
  auto glmPoints = points | std::ranges::views::transform([](auto point) {
                     return point->getPosition();
                   });
  _glmPoints.clear();
  std::copy(glmPoints.begin(), glmPoints.end(), std::back_inserter(_glmPoints));
}

bool BezierBuilder::drawPropertiesGui() {
  ImGui::Text("Bezier Curve Power: %d", bezierPower);
  return false;
}

/////////////////////////////////////
/// RationalBezierBuilder
/////////////////////////////////////

std::vector<float> RationalBezierBuilder::_allBernstein(float u) {
  std::vector<float> B(bezierPower + 1, 0);
  B[0] = 1.0f;
  float u1 = 1.0f - u;
  for (int j = 1; j <= bezierPower; j++) {
    float saved = 0.0f;
    for (int k = 0; k < j; k++) {
      float temp = B[k];
      B[k] = saved + u1 * temp;
      saved = u * temp;
    }
    B[j] = saved;
  }
  return B;
}

sptr<Point> RationalBezierBuilder::_deCasteljau(float u) {
  std::vector<glm::vec3> _glmPointsCopy;
  std::copy(_glmPoints.begin(), _glmPoints.end(),
            std::back_inserter(_glmPointsCopy));
  for (auto i = 0; i < _glmPointsCopy.size(); i++) {
    _glmPointsCopy[i] *= w[i];
  }
  for (int k = 1; k <= bezierPower; k++) {
    for (int i = 0; i < bezierPower - k + 1; i++) {
      _glmPointsCopy[i] =
          (1.0f - u) * _glmPointsCopy[i] + u * _glmPointsCopy[i + 1];
    }
  }
  return Point::create(_glmPointsCopy[0]);
}

RationalBezierBuilder::RationalBezierBuilder(
    const std::vector<sptr<Point>> &points, int bezierPower,
    const std::vector<float> &weights)
    : SplineBuilder(points), bezierPower(bezierPower), w(weights) {
  if (weights.size() < points.size()) {
    spdlog::warn(
        "When creating RationalBezierBuilder weight vector is smaller then "
        "points vector. Initializing weights vector with 1.");
    w = std::vector<float>(points.size(), 1);
  }
}

uptr<SplineBuilder> RationalBezierBuilder::clone() {
  std::vector<sptr<Point>> pointsCopy;
  for (auto point : points) {
    pointsCopy.push_back(point->clone());
  }
  return uptr<RationalBezierBuilder>(
      new RationalBezierBuilder(pointsCopy, bezierPower, w));
}

sptr<Point> RationalBezierBuilder::getSplinePoint(float t) {
  auto C = _deCasteljau(t);
  auto B = _allBernstein(t);
  float H = 0;
  for (int i = 0; i < w.size(); i++) {
    H += w[i] * B[i];
  }
  C->getPosition().x /= H;
  C->getPosition().z /= H;
  return C;
}

void RationalBezierBuilder::rebuild() {
  bezierPower = points.size() - 1;
  auto glmPoints = points | std::ranges::views::transform([](auto point) {
                     return point->getPosition();
                   });
  _glmPoints.clear();
  std::copy(glmPoints.begin(), glmPoints.end(), std::back_inserter(_glmPoints));
}

bool RationalBezierBuilder::drawPropertiesGui() {
  ImGui::Text("Bezier Curve Power: %d", bezierPower);
  bool updated = false;
  if (ImGui::TreeNode("Point Weights")) {
    ImGui::BeginGroup();
    const bool child_is_visible = ImGui::BeginChild("pefe", {0, 200});

    if (child_is_visible) {
      auto i = 0;
      for (auto &_w : w) {
        auto point_name = std::string("Point_") + std::to_string(i) + "_w";
        if (ImGui::DragFloat(point_name.c_str(), &_w, 0.05, 0, 5)) {
          updated = true;
        }
        i++;
      }
    }
    ImGui::EndChild();
    ImGui::EndGroup();

    ImGui::TreePop();
  }
  return updated;
}

/////////////////////////////////////
/// BSplineBuilder
/////////////////////////////////////

void BSplineBuilder::_checkAndSetDefault() {
  if (points.size() < bSplinePower + 1) {
    spdlog::warn("BSpline: When creating BSplineBuilder, not enough controls "
                 "points(len = {}) to create BSpline(power = {}). Should be "
                 "len(points) >= degree + 1. Changing power to len(points)-1. ",
                 points.size(), bSplinePower);
    bSplinePower = points.size() - 1;
  }
  if (knotVector.size() != points.size() + bSplinePower + 1) {
    spdlog::warn("BSpline: When creating BSplineBuilder, knotVector has not "
                 "enough elements. Should be len(points)+bSplinePower+1. "
                 "Initializing with default one.");
    this->knotVector = std::vector<float>(points.size() + bSplinePower + 1, 0);
    for (auto i = 0; i < bSplinePower + 1; i++) {
      this->knotVector[i] = 0;
      this->knotVector[knotVector.size() - i - 1] = 1;
    }
    float step =
        1.0f / ((float)knotVector.size() - 2.0f * bSplinePower - 2.0f + 1.0f);
    for (auto i = bSplinePower + 1;
         i < this->knotVector.size() - bSplinePower - 1; i++) {
      this->knotVector[i] = (i - bSplinePower) * step;
    }
    std::string vectorData = "{";
    for (auto e : this->knotVector) {
      vectorData += std::to_string(e) + " ";
    }
    vectorData += "}";
    spdlog::warn("BSpline: Default knot vector: {}", vectorData);
  }
}

BSplineBuilder::BSplineBuilder(const std::vector<sptr<Point>> &points,
                               int bSplinePower,
                               const std::vector<float> &knotVector = {})
    : SplineBuilder(points), bSplinePower(bSplinePower),
      knotVector(knotVector) {
  _checkAndSetDefault();
}

uptr<SplineBuilder> BSplineBuilder::clone() {
  std::vector<sptr<Point>> pointsCopy;
  for (auto point : points) {
    pointsCopy.push_back(point->clone());
  }
  return uptr<BSplineBuilder>(
      new BSplineBuilder(pointsCopy, bSplinePower, knotVector));
}

sptr<Point> BSplineBuilder::getSplinePoint(float t) {
  int span = bSplineFindSpan(points.size() - 1, bSplinePower, t, knotVector);
  auto N = bSplineBasisFunc(span, t, bSplinePower, knotVector);
  auto C = Point::create({0, 0, 0});
  for (int i = 0; i <= bSplinePower; i++) {
    *C = *C + N[i] * (*points[span - bSplinePower + i]);
  }
  return C;
}

std::vector<sptr<Point>> BSplineBuilder::getSplineDerivatives(float t,
                                                              int dirsCount) {
  int du = std::min(dirsCount, bSplinePower);
  std::vector<sptr<Point>> result;
  int span = bSplineFindSpan(points.size() - 1, bSplinePower, t, knotVector);
  auto nders = dersBasisFunc(span, t, bSplinePower, du, knotVector);
  for (auto k = 0; k <= du; k++) {
    auto C = Point::create({0, 0, 0});
    for (auto j = 0; j <= bSplinePower; j++) {
      *C = *C + nders[k][j] * *points[span - bSplinePower + j];
    }
    result.push_back(C);
  }
  return result;
}

void BSplineBuilder::rebuild() { _checkAndSetDefault(); }

bool BSplineBuilder::drawPropertiesGui() {
  bool modified = false;
  if (ImGui::InputInt("BSpline Degree", &bSplinePower))
    modified = true;
  bool updated = false;
  if (ImGui::TreeNode("Knot Vector")) {
    ImGui::BeginGroup();
    const bool child_is_visible = ImGui::BeginChild("pefe", {0, 200});
    if (child_is_visible) {
      for (auto i = 0; i < knotVector.size(); i++) {
        std::string label = "Knot_" + std::to_string(i);
        if (i < bSplinePower + 1 || i >= knotVector.size() - bSplinePower - 1) {
          ImGui::BeginDisabled();
          ImGui::SliderFloat(label.c_str(), &knotVector[i], 0, 1, "%.3f");
          ImGui::EndDisabled();

        } else {
          if (ImGui::SliderFloat(label.c_str(), &knotVector[i],
                                 knotVector[i - 1], knotVector[i + 1])) {
            modified = true;
          }
        }
      }
    }
    ImGui::EndChild();
    ImGui::EndGroup();

    ImGui::TreePop();
  }

  return modified;
}

/////////////////////////////////////
/// RationalBSplineBuilder
/////////////////////////////////////

void RationalBSplineBuilder::_checkAndSetDefault() {
  if (points.size() < bSplinePower + 1) {
    spdlog::warn("RationalBSpline: When creating BSplineBuilder, not enough "
                 "controls "
                 "points(len = {}) to create BSpline(power = {}). Should be "
                 "len(points) >= degree + 1. Changing power to len(points)-1. ",
                 points.size(), bSplinePower);
    bSplinePower = points.size() - 1;
  }
  if (knotVector.size() != points.size() + bSplinePower + 1) {
    spdlog::warn(
        "RationalBSpline: When creating BSplineBuilder, knotVector has not "
        "enough elements. Should be len(points)+bSplinePower+1. "
        "Initializing "
        "with default one.");
    this->knotVector = std::vector<float>(points.size() + bSplinePower + 1, 0);
    for (auto i = 0; i < bSplinePower + 1; i++) {
      this->knotVector[i] = 0;
      this->knotVector[knotVector.size() - i - 1] = 1;
    }
    float step =
        1.0f / ((float)knotVector.size() - 2.0f * bSplinePower - 2.0f + 1.0f);
    for (auto i = bSplinePower + 1;
         i < this->knotVector.size() - bSplinePower - 1; i++) {
      this->knotVector[i] = (i - bSplinePower) * step;
    }
    std::string vectorData = "{";
    for (auto e : this->knotVector) {
      vectorData += std::to_string(e) + " ";
    }
    vectorData += "}";
    spdlog::warn("RationalBSpline: Default knot vector: {}", vectorData);
  }
  if (weights.size() != points.size()) {
    spdlog::warn("RationalBSpline: Weights count not equal to control points "
                 "count. Initializing with default one");
    weights = std::vector<float>(points.size(), 1);
  }
}

RationalBSplineBuilder::RationalBSplineBuilder(
    const std::vector<sptr<Point>> &points, int bSplinePower,
    const std::vector<float> &knotVector = {},
    const std::vector<float> &weights = {})
    : SplineBuilder(points), bSplinePower(bSplinePower), knotVector(knotVector),
      weights(weights) {
  _checkAndSetDefault();
}

uptr<SplineBuilder> RationalBSplineBuilder::clone() {
  std::vector<sptr<Point>> pointsCopy;
  for (auto point : points) {
    pointsCopy.push_back(point->clone());
  }
  return uptr<RationalBSplineBuilder>(new RationalBSplineBuilder(
      pointsCopy, bSplinePower, knotVector, weights));
}

sptr<Point> RationalBSplineBuilder::getSplinePoint(float t) {
  // int span = bSplineFindSpan(points.size() - 1, bSplinePower, t,
  // knotVector); auto N = bSplineBasisFunc(span, t, bSplinePower,
  // knotVector); glm::vec4 C = {0, 0, 0, 0}; float H = 0.0f;
  //
  // for (int i = 0; i <= bSplinePower; i++) {
  //   C = C + N[i] * (_glmPoints[span - bSplinePower + i]);
  //   H += weights[span - bSplinePower + i] * N[i];
  // }
  // return Point::create({C.x / H, C.y / H, C.z / H});

  auto ck = getSplineDerivatives(t, 2);
  return ck[0];
}

std::vector<sptr<Point>>
RationalBSplineBuilder::getSplineDerivatives(float t, int dirsCount) {
  int du = std::min(dirsCount, bSplinePower);
  std::vector<glm::vec4> c4d;
  int span = bSplineFindSpan(points.size() - 1, bSplinePower, t, knotVector);
  auto nders = dersBasisFunc(span, t, bSplinePower, du, knotVector);
  for (auto k = 0; k <= du; k++) {
    glm::vec4 C = {0, 0, 0, 0};
    for (auto j = 0; j <= bSplinePower; j++) {
      C = C + nders[k][j] * _glmPoints[span - bSplinePower + j];
    }
    c4d.push_back(C);
  }
  std::vector<glm::vec3> aders;
  std::vector<float> wders;

  for (auto &p : c4d) {
    aders.push_back({p.x, p.y, p.z});
    wders.push_back(p.w);
  }

  std::vector<glm::vec3> CK;

  for (auto k = 0; k <= du; k++) {
    auto v = aders[k];
    for (auto i = 1; i <= k; i++) {
      v = v - binomialCoeff(k, i) * wders[i] * CK[k - i];
    }
    CK.push_back(v / wders[0]);
  }

  std::vector<sptr<Point>> result;
  for (auto &p : CK) {
    result.push_back(Point::create(p));
  }
  return result;
}

void RationalBSplineBuilder::rebuild() {
  _checkAndSetDefault();
  _glmPoints.clear();
  int i = 0;
  for (auto point : points) {
    auto pos = point->getPosition();
    _glmPoints.push_back(glm::vec4{pos * weights[i], weights[i]});
    i++;
  }
}

bool RationalBSplineBuilder::drawPropertiesGui() {
  bool modified = false;

  if (ImGui::InputInt("BSpline Degree", &bSplinePower))
    modified = true;
  if (ImGui::TreeNode("Point Weights")) {
    ImGui::BeginGroup();
    const bool child_is_visible = ImGui::BeginChild("pefe", {0, 200});

    if (child_is_visible) {
      auto i = 0;
      for (auto &_w : weights) {
        auto point_name = std::string("Point_") + std::to_string(i) + "_w";
        if (ImGui::DragFloat(point_name.c_str(), &_w, 0.05, 0, 5)) {
          modified = true;
        }
        i++;
      }
    }
    ImGui::EndChild();
    ImGui::EndGroup();

    ImGui::TreePop();
  }
  if (ImGui::TreeNode("Knot Vector")) {
    ImGui::BeginGroup();
    const bool child_is_visible = ImGui::BeginChild("pefe", {0, 200});
    if (child_is_visible) {
      for (auto i = 0; i < knotVector.size(); i++) {
        std::string label = "Knot_" + std::to_string(i);
        if (i < bSplinePower + 1 || i >= knotVector.size() - bSplinePower - 1) {
          ImGui::BeginDisabled();
          ImGui::SliderFloat(label.c_str(), &knotVector[i], 0, 1, "%.3f");
          ImGui::EndDisabled();

        } else {
          if (ImGui::DragFloat(label.c_str(), &knotVector[i], 0.01,
                               knotVector[i - 1], knotVector[i + 1])) {
            modified = true;
          }
        }
      }
    }
    ImGui::EndChild();
    ImGui::EndGroup();

    ImGui::TreePop();
  }

  return modified;
}

} // namespace EGEOM
