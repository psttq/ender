#pragma once
#include <string>
#include <Spline1.hpp>

namespace EGEOM{
    class Sketch{
        sptr<Spline1> _spline;

        Sketch(const std::string &name, sptr<Spline1> spline);
    public:
        std::string name;

        static sptr<Sketch> create(const std::string &name, sptr<Spline1> spline);

        sptr<Spline1> getSpline();
        void setSpline(sptr<Spline1> spline);
    };
}