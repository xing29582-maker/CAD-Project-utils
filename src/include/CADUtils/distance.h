#pragma once
#include <cmath>

namespace cadutils {

    inline double distance2D(double x1, double y1, double x2, double y2)
    {
        const double dx = x1 - x2;
        const double dy = y1 - y2;
        return std::sqrt(dx * dx + dy * dy);
    }

} // namespace cadutils
