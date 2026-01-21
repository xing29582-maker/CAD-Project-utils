#include "Vector3d.h"

#include <cmath> 

using namespace cadutils;

cadutils::Vector3d::Vector3d(double x, double y, double z)
	:m_x(x)
	,m_y(y)
	,m_z(z)
{
}

cadutils::Vector3d::~Vector3d()
{
}

void cadutils::Vector3d::SetX(double x)
{
	m_x = x;
}

void cadutils::Vector3d::SetY(double y)
{
	m_y = y;
}

void cadutils::Vector3d::SetZ(double z)
{
	m_z = z;
}

double cadutils::Vector3d::GetX() const
{
	return m_x;
}

double cadutils::Vector3d::GetY() const
{
	return m_y;
}

double cadutils::Vector3d::GetZ() const
{
	return m_z;
}

double Vector3d::LengthSq() const
{
    return m_x * m_x + m_y * m_y + m_z * m_z;
}

double Vector3d::Length() const
{
    return std::sqrt(LengthSq());
}

Vector3d Vector3d::Normalized(double eps) const
{
    const double lenSq = LengthSq();
    if (lenSq <= eps * eps)
        return Vector3d(0.0, 0.0, 0.0);

    const double invLen = 1.0 / std::sqrt(lenSq);
    return Vector3d(m_x * invLen, m_y * invLen, m_z * invLen);
}

bool Vector3d::Normalize(double eps)
{
    const double lenSq = LengthSq();
    if (lenSq <= eps * eps)
        return false;

    const double invLen = 1.0 / std::sqrt(lenSq);
    m_x *= invLen;
    m_y *= invLen;
    m_z *= invLen;
    return true;
}

double Vector3d::Dot(const Vector3d& rhs) const
{
    return m_x * rhs.m_x + m_y * rhs.m_y + m_z * rhs.m_z;
}

Vector3d Vector3d::Cross(const Vector3d& rhs) const
{
    return Vector3d(
        m_y * rhs.m_z - m_z * rhs.m_y,
        m_z * rhs.m_x - m_x * rhs.m_z,
        m_x * rhs.m_y - m_y * rhs.m_x
    );
}

Vector3d Vector3d::operator+(const Vector3d& rhs) const
{
    return Vector3d(m_x + rhs.m_x, m_y + rhs.m_y, m_z + rhs.m_z);
}

Vector3d Vector3d::operator-(const Vector3d& rhs) const
{
    return Vector3d(m_x - rhs.m_x, m_y - rhs.m_y, m_z - rhs.m_z);
}

Vector3d Vector3d::operator-() const
{
    return Vector3d(-m_x, -m_y, -m_z);
}

Vector3d& Vector3d::operator+=(const Vector3d& rhs)
{
    m_x += rhs.m_x; m_y += rhs.m_y; m_z += rhs.m_z;
    return *this;
}

Vector3d& Vector3d::operator-=(const Vector3d& rhs)
{
    m_x -= rhs.m_x; m_y -= rhs.m_y; m_z -= rhs.m_z;
    return *this;
}

Vector3d Vector3d::operator*(double s) const
{
    return Vector3d(m_x * s, m_y * s, m_z * s);
}

Vector3d Vector3d::operator/(double s) const
{
    // 你也可以选择不检查，或者用 assert
    if (std::fabs(s) <= DBL_EPSILON)
        return Vector3d(0.0, 0.0, 0.0);
    const double inv = 1.0 / s;
    return Vector3d(m_x * inv, m_y * inv, m_z * inv);
}

Vector3d& Vector3d::operator*=(double s)
{
    m_x *= s; m_y *= s; m_z *= s;
    return *this;
}

Vector3d& Vector3d::operator/=(double s)
{
    if (std::fabs(s) <= DBL_EPSILON)
    {
        m_x = m_y = m_z = 0.0;
        return *this;
    }
    const double inv = 1.0 / s;
    m_x *= inv; m_y *= inv; m_z *= inv;
    return *this;
}

double Vector3d::operator*(const Vector3d& rhs) const
{
    // 点乘：v*w
    return Dot(rhs);
}

Vector3d Vector3d::operator^(const Vector3d& rhs) const
{
    // 叉乘：v^w
    return Cross(rhs);
}