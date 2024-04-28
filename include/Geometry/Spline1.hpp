#pragma once

#include <Ender.hpp>
#include <Point.hpp>

namespace EGEOM {

    class SplineBuilder {
    public:
        SplineBuilder(const std::vector<sptr<Point>> &points) : points(points) {};
        std::vector<sptr<Point>> points;

        virtual sptr<Point> getSplinePoint(float t) = 0;

        virtual void rebuild() = 0;

        virtual void drawPropertiesGui() = 0;
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

        void drawPropertiesGui() override{
            std::vector<const char *> items = {
                    "Uniform",
            };
            int currentItem = static_cast<int>(paramMethod);

            if(ImGui::Combo("Parameter Calculation Method", &currentItem,&items[0],items.size())){}

            if (ImGui::TreeNode("Parameter")) {
                ImGui::BeginGroup();
                const bool child_is_visible = ImGui::BeginChild("pefe", {0, 200});

                if (child_is_visible) {
                    auto i = 0;
                    for (auto t: _t) {
                        auto point_name = std::string("Point_") + std::to_string(i) + "_t";
                        ImGui::InputFloat(point_name.c_str(), &t,0, ImGuiInputTextFlags_ReadOnly);
                        i++;
                    }
                }
                ImGui::EndChild();
                ImGui::EndGroup();

                ImGui::TreePop();
            }
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
            std::vector<float> B(bezierPower+1, 0);
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

    public:
        int bezierPower = 3;

        BezierBuilder(const std::vector<sptr<Point>> &points, int bezierPower) : SplineBuilder(points),
                                                                                 bezierPower(bezierPower) {
        }

        sptr<Point> getSplinePoint(float t) override {
            auto B = _allBernstein(t);
            auto C = Point::create({0, 0, 0});
            for (int k = 0; k <= bezierPower; k++) {
                *C = *C + B[k] * (*(points[k]));
            }
            return C;
        }

        void rebuild() override {
            bezierPower = points.size()-1;
        }

        void drawPropertiesGui() override {
            ImGui::Text("Bezier Curve Power: %d", bezierPower);
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

        void setInterpolationPointsCount(uint count);

        void setSplineType(SplineType splineType);

        void setSplineBuilder(uptr<SplineBuilder> splineBuilder);

        void getPropertiesGUI(bool scrollToPoint);

        void update();
    };
} // namespace EGEOM
