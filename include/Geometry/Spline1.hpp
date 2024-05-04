#pragma once

#include <Ender.hpp>
#include <Point.hpp>
#include <ranges>

namespace EGEOM {

    class SplineBuilder {
    public:
        SplineBuilder(const std::vector<sptr<Point>> &points) : points(points) {};
        std::vector<sptr<Point>> points;

        virtual ~SplineBuilder() = default;

        virtual sptr<Point> getSplinePoint(float t) = 0;

        virtual void rebuild() = 0;

        virtual bool drawPropertiesGui() = 0;
    };

    class LinearInterpolationBuilder : public SplineBuilder {
    public:
        enum class ParamMethod : int {
            Uniform, Chordal, Centripetal
        };

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

        bool drawPropertiesGui() override {
            std::vector<const char *> items = {
                    "Uniform",
            };
            int currentItem = static_cast<int>(paramMethod);

            if (ImGui::Combo("Parameter Calculation Method", &currentItem, &items[0], items.size())) {}

            if (ImGui::TreeNode("Parameter")) {
                ImGui::BeginGroup();
                const bool child_is_visible = ImGui::BeginChild("pefe", {0, 200});

                if (child_is_visible) {
                    auto i = 0;
                    for (auto t: _t) {
                        auto point_name = std::string("Point_") + std::to_string(i) + "_t";
                        ImGui::InputFloat(point_name.c_str(), &t, 0, 0.0f,"%.3f", ImGuiInputTextFlags_ReadOnly);
                        i++;
                    }
                }
                ImGui::EndChild();
                ImGui::EndGroup();

                ImGui::TreePop();
            }
            return false;

        }

    private:
        void calculateParameter() {
            switch (paramMethod) {
                case ParamMethod::Uniform: {
                    calculateUniformParameter();
                }
                    break;
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


    class BezierBuilder : public SplineBuilder {
        std::vector<float> _allBernstein(float u) {
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

        sptr<Point> _deCasteljau(float u) {
            std::vector<glm::vec3> _glmPointsCopy;
            std::copy(_glmPoints.begin(), _glmPoints.end(), std::back_inserter(_glmPointsCopy));
            for (int k = 1; k <= bezierPower; k++) {
                for (int i = 0; i < bezierPower - k + 1; i++) {
                    _glmPointsCopy[i] = (1.0f - u) * _glmPointsCopy[i] + u * _glmPointsCopy[i + 1];
                }
            }
            return Point::create(_glmPointsCopy[0]);
        }

        sptr<Point> pointWithAllBernstein(float u) {
            auto B = _allBernstein(u);
            auto C = Point::create({0, 0, 0});
            for (int k = 0; k <= bezierPower; k++) {
                *C = *C + B[k] * (*(points[k]));
            }
            return C;
        }

        std::vector<glm::vec3> _glmPoints;
    public:
        int bezierPower = 3;
        bool useDeCasteljau = true;

        BezierBuilder(const std::vector<sptr<Point>> &points, int bezierPower) : SplineBuilder(points),
                                                                                 bezierPower(bezierPower) {
        }

        sptr<Point> getSplinePoint(float t) override {
            if (useDeCasteljau) {
                return _deCasteljau(t);
            }
            return pointWithAllBernstein(t);
        }

        void rebuild() override {
            bezierPower = points.size() - 1;
            auto glmPoints = points | std::ranges::views::transform([](auto point) {
                return point->getPosition();
            });
            _glmPoints.clear();
            std::copy(glmPoints.begin(), glmPoints.end(), std::back_inserter(_glmPoints));
        }

        bool drawPropertiesGui() override {
            ImGui::Text("Bezier Curve Power: %d", bezierPower);
            return false;
        }
    };

    class RationalBezierBuilder : public SplineBuilder {
    private:
        std::vector<glm::vec3> _glmPoints;

        std::vector<float> _allBernstein(float u) {
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

        sptr<Point> _deCasteljau(float u) {
            std::vector<glm::vec3> _glmPointsCopy;
            std::copy(_glmPoints.begin(), _glmPoints.end(), std::back_inserter(_glmPointsCopy));
            for(auto i = 0; i < _glmPointsCopy.size(); i++){
                _glmPointsCopy[i]*=w[i];
            }
            for (int k = 1; k <= bezierPower; k++) {
                for (int i = 0; i < bezierPower - k + 1; i++) {
                    _glmPointsCopy[i] = (1.0f - u) * _glmPointsCopy[i] + u * _glmPointsCopy[i + 1];
                }
            }
            return Point::create(_glmPointsCopy[0]);
        }

    public:
        std::vector<float> w;
        int bezierPower = 3;

        RationalBezierBuilder(const std::vector<sptr<Point>> &points, int bezierPower,
                              const std::vector<float> &weights) : SplineBuilder(points), bezierPower(bezierPower),
                                                                   w(weights) {
            if (weights.size() < points.size()) {
                spdlog::warn(
                        "When creating RationalBezierBuilder weight vector is smaller then points vector. Initializing weights vector with 1.");
                w = std::vector<float>(points.size(), 1);
            }
        }

        sptr<Point> getSplinePoint(float t) override {
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

        void rebuild() override {
            bezierPower = points.size() - 1;
            auto glmPoints = points | std::ranges::views::transform([](auto point) {
                return point->getPosition();
            });
            _glmPoints.clear();
            std::copy(glmPoints.begin(), glmPoints.end(), std::back_inserter(_glmPoints));
        }

        bool drawPropertiesGui() override {
            ImGui::Text("Bezier Curve Power: %d", bezierPower);
            bool updated = false;
            if (ImGui::TreeNode("Point Weights")) {
                ImGui::BeginGroup();
                const bool child_is_visible = ImGui::BeginChild("pefe", {0, 200});

                if (child_is_visible) {
                    auto i = 0;
                    for (auto &_w: w) {
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
    };

    class BSplineBuilder : public SplineBuilder {
        int _findSpan(int n, int p, float u, std::vector<float> U) {
            if (u == U[n + 1]) {
                return n;
            }
            int low = p;
            int high = n + 1;
            int mid = (low + high) / 2;
            while (u < U[mid] || u >= U[mid + 1]) {
                if (u < U[mid])
                    high = mid;
                else
                    low = mid;
                mid = (low + high) / 2;
            }
            return mid;
        }

        std::vector<float> _basisFunc(int i, float u, int p, std::vector<float> U) {
            std::vector<float> N(p + 1, 0);
            std::vector<float> left(p + 1, 0);
            std::vector<float> right(p + 1, 0);

            N[0] = 1.0f;
            for (int j = 1; j <= p; j++) {
                left[j] = u - U[i + 1 - j];
                right[j] = U[i + j] - u;
                float saved = 0.0f;
                for(int r = 0; r < j; r++){
                    float temp = N[r]/(right[r+1]+left[j-r]);
                    N[r] = saved+right[r+1]*temp;
                    saved = left[j-r]*temp;
                }
                N[j] = saved;
            }
            return N;
        }

        void _checkAndSetDefault(){
            if (points.size() < bSplinePower + 1) {
                spdlog::warn(
                        "BSpline: When creating BSplineBuilder, not enough controls points(len = {}) to create BSpline(power = {}). Should be len(points) >= degree + 1. Changing power to len(points)-1. ",
                        points.size(), bSplinePower);
                bSplinePower = points.size() - 1;
            }
            if (knotVector.size() != points.size() + bSplinePower + 1) {
                spdlog::warn(
                        "BSpline: When creating BSplineBuilder, knotVector has not enough elements. Should be len(points)+bSplinePower+1. Initializing with default one.");
                this->knotVector = std::vector<float>(points.size() + bSplinePower + 1, 0);
                for (auto i = 0; i < bSplinePower + 1; i++) {
                    this->knotVector[i] = 0;
                    this->knotVector[knotVector.size() - i- 1] = 1;
                }
                float step = 1.0f / ((float)knotVector.size() - 2.0f * bSplinePower - 2.0f + 1.0f);
                for (auto i = bSplinePower + 1; i < this->knotVector.size() - bSplinePower - 1; i++) {
                    spdlog::info("here {} {}", i, step);
                    this->knotVector[i] = (i-bSplinePower) * step;
                }
                std::string vectorData = "{";
                for (auto e: this->knotVector) {
                    vectorData += std::to_string(e) + " ";
                }
                vectorData += "}";
                spdlog::warn("BSpline: Default knot vector: {}", vectorData);
            }
        }

    public:
        int bSplinePower = 0;
        std::vector<float> knotVector = {};

        BSplineBuilder(const std::vector<sptr<Point>> &points, int bSplinePower,
                       const std::vector<float> &knotVector = {}) : SplineBuilder(points), bSplinePower(bSplinePower),
                                                                    knotVector(knotVector) {
            _checkAndSetDefault();
        }

        sptr<Point> getSplinePoint(float t) override {
            int span = _findSpan(points.size()-1, bSplinePower,t,knotVector);
            auto N = _basisFunc(span, t,bSplinePower,knotVector);
            auto C = Point::create({0,0,0});
            for(int i = 0; i <= bSplinePower; i++){
                *C = *C+N[i]*(*points[span-bSplinePower+i]);
            }
            return C;
        }

        void rebuild() override{
            _checkAndSetDefault();
        }

        bool drawPropertiesGui() override{
            bool modified = false;
            if(ImGui::InputInt("BSpline Degree",&bSplinePower))
                modified = true;
            bool updated = false;
            if (ImGui::TreeNode("Knot Vector")) {
                ImGui::BeginGroup();
                const bool child_is_visible = ImGui::BeginChild("pefe", {0, 200});
                if (child_is_visible) {
                    for(auto i = 0; i < knotVector.size(); i++){
                        std::string label = "Knot_" + std::to_string(i);
                        if(i < bSplinePower+1 || i >= knotVector.size() - bSplinePower -1){
                            ImGui::BeginDisabled();
                            ImGui::SliderFloat(label.c_str(), &knotVector[i],0,1,"%.3f");
                            ImGui::EndDisabled();

                        }
                        else {
                            if (ImGui::SliderFloat(label.c_str(), &knotVector[i], knotVector[i-1], knotVector[i+1])) {
                                modified = true;
                                spdlog::info(knotVector[i]);
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
    };

    class RationalBSplineBuilder : public SplineBuilder {
        int _findSpan(int n, int p, float u, std::vector<float> U) {
            if (u == U[n + 1]) {
                return n;
            }
            int low = p;
            int high = n + 1;
            int mid = (low + high) / 2;
            while (u < U[mid] || u >= U[mid + 1]) {
                if (u < U[mid])
                    high = mid;
                else
                    low = mid;
                mid = (low + high) / 2;
            }
            return mid;
        }

        std::vector<float> _basisFunc(int i, float u, int p, std::vector<float> U) {
            std::vector<float> N(p + 1, 0);
            std::vector<float> left(p + 1, 0);
            std::vector<float> right(p + 1, 0);

            N[0] = 1.0f;
            for (int j = 1; j <= p; j++) {
                left[j] = u - U[i + 1 - j];
                right[j] = U[i + j] - u;
                float saved = 0.0f;
                for(int r = 0; r < j; r++){
                    float temp = N[r]/(right[r+1]+left[j-r]);
                    N[r] = saved+right[r+1]*temp;
                    saved = left[j-r]*temp;
                }
                N[j] = saved;
            }
            return N;
        }

        void _checkAndSetDefault(){
            if (points.size() < bSplinePower + 1) {
                spdlog::warn(
                        "RationalBSpline: When creating BSplineBuilder, not enough controls points(len = {}) to create BSpline(power = {}). Should be len(points) >= degree + 1. Changing power to len(points)-1. ",
                        points.size(), bSplinePower);
                bSplinePower = points.size() - 1;
            }
            if (knotVector.size() != points.size() + bSplinePower + 1) {
                spdlog::warn(
                        "RationalBSpline: When creating BSplineBuilder, knotVector has not enough elements. Should be len(points)+bSplinePower+1. Initializing with default one.");
                this->knotVector = std::vector<float>(points.size() + bSplinePower + 1, 0);
                for (auto i = 0; i < bSplinePower + 1; i++) {
                    this->knotVector[i] = 0;
                    this->knotVector[knotVector.size() - i- 1] = 1;
                }
                float step = 1.0f / ((float)knotVector.size() - 2.0f * bSplinePower - 2.0f + 1.0f);
                for (auto i = bSplinePower + 1; i < this->knotVector.size() - bSplinePower - 1; i++) {
                    spdlog::info("here {} {}", i, step);
                    this->knotVector[i] = (i-bSplinePower) * step;
                }
                std::string vectorData = "{";
                for (auto e: this->knotVector) {
                    vectorData += std::to_string(e) + " ";
                }
                vectorData += "}";
                spdlog::warn("RationalBSpline: Default knot vector: {}", vectorData);
            }
            if(weights.size() != points.size()){
                spdlog::warn("RationalBSpline: Weights count not equal to control points count. Initializing with default one");
                weights = std::vector<float>(points.size(), 1);
            }
        }
        std::vector<glm::vec3> _glmPoints;

    public:
        int bSplinePower = 0;
        std::vector<float> knotVector = {};
        std::vector<float> weights = {};

        RationalBSplineBuilder(const std::vector<sptr<Point>> &points, int bSplinePower,
                       const std::vector<float> &knotVector = {}, const std::vector<float> &weights = {}) : SplineBuilder(points), bSplinePower(bSplinePower),
                                                                    knotVector(knotVector), weights(weights) {
            _checkAndSetDefault();
        }

        sptr<Point> getSplinePoint(float t) override {
            int span = _findSpan(points.size()-1, bSplinePower,t,knotVector);
            auto N = _basisFunc(span, t,bSplinePower,knotVector);
            glm::vec3 C = {0,0,0};
            float H = 0.0f;

            for(int i = 0; i <= bSplinePower; i++){
                C = C+N[i]*(_glmPoints[span-bSplinePower+i]);
                H += weights[span-bSplinePower+i] * N[i];
            }
            return Point::create({C.x/H, 0, C.z/H});
        }

        void rebuild() override{
            _checkAndSetDefault();
            auto glmPoints = points | std::ranges::views::transform([](auto point) {
                return point->getPosition();
            });
            _glmPoints.clear();
            std::copy(glmPoints.begin(), glmPoints.end(), std::back_inserter(_glmPoints));
            for(auto i = 0; i < _glmPoints.size(); i++){
                _glmPoints[i]*=weights[i];
            }
        }

        bool drawPropertiesGui() override{
            bool modified = false;
            if(ImGui::InputInt("BSpline Degree",&bSplinePower))
                modified = true;
            if (ImGui::TreeNode("Point Weights")) {
                ImGui::BeginGroup();
                const bool child_is_visible = ImGui::BeginChild("pefe", {0, 200});

                if (child_is_visible) {
                    auto i = 0;
                    for (auto &_w: weights) {
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
                    for(auto i = 0; i < knotVector.size(); i++){
                        std::string label = "Knot_" + std::to_string(i);
                        if(i < bSplinePower+1 || i >= knotVector.size() - bSplinePower -1){
                            ImGui::BeginDisabled();
                            ImGui::SliderFloat(label.c_str(), &knotVector[i],0,1,"%.3f");
                            ImGui::EndDisabled();

                        }
                        else {
                            if (ImGui::DragFloat(label.c_str(), &knotVector[i], 0.01, knotVector[i-1],knotVector[i+1])) {
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
    };



    class Spline1 : public ENDER::Object {
    public:
        enum class ParamMethod {
            Uniform, Chordal, Centripetal
        };

        enum class SplineType : int {
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

        glm::vec3 getSplinePoint(float u);

        void setInterpolationPointsCount(uint count);

        void setSplineType(SplineType splineType);

        void setSplineBuilder(uptr<SplineBuilder> splineBuilder);

        void getPropertiesGUI(bool scrollToPoint);

        void update();
    };
} // namespace EGEOM
