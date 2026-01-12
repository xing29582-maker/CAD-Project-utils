#pragma once

#include "GeometryExport.h"
#include <memory>

namespace cadutils
{
	class CADUTILS_GEOMETRY_API Point3d
	{
	public:
        Point3d(double x, double y, double z);
        ~Point3d();

        void SetX(double x);
        void SetY(double y);
        void SetZ(double z);

        double GetX() const;
        double GetY() const;
        double GetZ() const;

    private:
        double m_x;
        double m_y;
        double m_z;
	};
}