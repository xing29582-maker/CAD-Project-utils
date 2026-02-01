#pragma once

#include "CommonExport.h"
#include "Vector3d.h"

#include <memory>

namespace cadutils
{
	class CADUTILS_COMMON_API Point3d
	{
	public:
        Point3d();
        Point3d(double x, double y, double z);
        ~Point3d();

        void SetX(double x);
        void SetY(double y);
        void SetZ(double z);

        double GetX() const;
        double GetY() const;
        double GetZ() const;

        // ---- 运算符重载（点-向量模型） ----

        // p2 - p1 = 向量
        Vector3d operator-(const Point3d& rhs) const;

        // p + v = p'
        Point3d operator+(const Vector3d& v) const;
        Point3d operator-(const Vector3d& v) const;

        Point3d& operator+=(const Vector3d& v);
        Point3d& operator-=(const Vector3d& v);

        // 几何等价（带容差）
        bool IsEqual(const Point3d& rhs, double eps = 1e-12) const;

        bool operator==(const Point3d& rhs) const;
        bool operator!=(const Point3d& rhs) const;
    private:
        double m_x;
        double m_y;
        double m_z;
	};
}