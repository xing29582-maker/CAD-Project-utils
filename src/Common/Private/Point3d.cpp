#include "Point3d.h"

#include <cmath> 

using namespace cadutils;

cadutils::Point3d::Point3d()
    :Point3d(0,0,0)
{
}

cadutils::Point3d::Point3d(double x, double y, double z)
	:m_x(x)
	,m_y(y)
	,m_z(z)
{
}

cadutils::Point3d::~Point3d()
{
}

void cadutils::Point3d::SetX(double x)
{
	m_x = x;
}

void cadutils::Point3d::SetY(double y)
{
	m_y = y;
}

void cadutils::Point3d::SetZ(double z)
{
	m_z = z;
}

double cadutils::Point3d::GetX() const
{
	return m_x;
}

double cadutils::Point3d::GetY() const
{
	return m_y;
}

double cadutils::Point3d::GetZ() const
{
	return m_z;
}

// p2 - p1 = vector
Vector3d Point3d::operator-(const Point3d& rhs) const
{
    return Vector3d(
        m_x - rhs.m_x,
        m_y - rhs.m_y,
        m_z - rhs.m_z
    );
}

// p + v
Point3d Point3d::operator+(const Vector3d& v) const
{
    return Point3d(
        m_x + v.GetX(),
        m_y + v.GetY(),
        m_z + v.GetZ()
    );
}

// p - v
Point3d Point3d::operator-(const Vector3d& v) const
{
    return Point3d(
        m_x - v.GetX(),
        m_y - v.GetY(),
        m_z - v.GetZ()
    );
}

Point3d& Point3d::operator+=(const Vector3d& v)
{
    m_x += v.GetX();
    m_y += v.GetY();
    m_z += v.GetZ();
    return *this;
}

Point3d& Point3d::operator-=(const Vector3d& v)
{
    m_x -= v.GetX();
    m_y -= v.GetY();
    m_z -= v.GetZ();
    return *this;
}

bool Point3d::IsEqual(const Point3d& rhs, double eps) const
{
    return std::fabs(m_x - rhs.m_x) <= eps &&
        std::fabs(m_y - rhs.m_y) <= eps &&
        std::fabs(m_z - rhs.m_z) <= eps;
}

bool Point3d::operator==(const Point3d& rhs) const
{
    return IsEqual(rhs);
}

bool Point3d::operator!=(const Point3d& rhs) const
{
    return !IsEqual(rhs);
}