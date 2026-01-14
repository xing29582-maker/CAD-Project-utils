#include "Point3d.h"


using namespace cadutils;

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
