#pragma once
#include <Ender.hpp>
#include <Point.hpp>

namespace EGEOM
{
    class Spline1 : public ENDER::Object
    {
    public:
        enum class ParamMethod
        {
            Uniform,
            Chordal,
            Centripetal
        };

    private:
        std::vector<sptr<Point>> _points;
        std::vector<float> _t;
        ParamMethod _paramMethod;
        uint _interpolatedPointsCount;
        std::vector<sptr<Point>> _interpolatedPoints;
        std::vector<float> _rawData;

        void _calculateParameter();
        void _calculateUniformParameter();

        void _calculateDrawPoints();

        Spline1(const std::vector<sptr<Point>> &points, ParamMethod paramMethod, uint interpolatedPointsCount);

    public:

        static sptr<Spline1> create(const std::vector<sptr<Point>> &points, ParamMethod paramMethod, uint interpolatedPointsCount);

        void addPoint(sptr<Point> point);
        void setPoints(const std::vector<sptr<Point>> &points);
        std::vector<sptr<Point>> getInterpolatedPoints();
        std::vector<sptr<Point>> getPoints();

        void setInterpolationPointsCount(uint count);

        void update();
    };
}