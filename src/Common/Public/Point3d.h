#pragma once

#include "CommonExport.h"
#include "Vector3d.h"
#include "NameDefine.h"

#include <memory>
#include <string>

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

        // ---- operator overloads (point-vector model) ----

        // p2 - p1 = vector
        Vector3d operator-(const Point3d& rhs) const;

        // p + v = p'
        Point3d operator+(const Vector3d& v) const;
        Point3d operator-(const Vector3d& v) const;

        Point3d& operator+=(const Vector3d& v);
        Point3d& operator-=(const Vector3d& v);

        // serialization support
        std::string ToString() const;
        static Point3d FromString(const std::string& s);

        // geometric equality (tolerance comparison)
        bool IsEqual(const Point3d& rhs, double eps = 1e-12) const;

        bool operator==(const Point3d& rhs) const;
        bool operator!=(const Point3d& rhs) const;
    private:
        double m_x;
        double m_y;
        double m_z;
 };

    // --- AnyValue <-> Point3d inline implementations ---
    // (Point3d is now complete, so we can define these)

    inline AnyValue::AnyValue(const Point3d& pt)
        : text(pt.ToString())
    {
    }

    template<>
    inline Point3d AnyValue::Get<Point3d>() const
    {
        return Point3d::FromString(text);
    }
}