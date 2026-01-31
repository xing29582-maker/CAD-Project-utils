#include "SphereObject.h"
#include "GeoBuildUtils.h"

using namespace cadutils;

cadutils::SphereObject::SphereObject(const std::string& name, const Point3d& center, double radius)
	:Object(name)
	, m_radius(radius)
	, m_center(center)
{
}

std::shared_ptr<IBody> cadutils::SphereObject::buildShape()
{
	return GeoBuildUtils::CreateSphere(m_center, m_radius);
}

const Point3d& cadutils::SphereObject::GetCenter() const
{
	return m_center;
}

double cadutils::SphereObject::GetRadius() const
{
	return m_radius;
}

void cadutils::SphereObject::SetRadius(double radius)
{
	m_radius = radius;
}

bool cadutils::SphereObject::SetParameters(ParamKey key, std::string value)
{
	switch (key)
	{
	case ParamKey::Radius:
		m_radius = std::stod(value);
		return true;
	case ParamKey::CenterX:
		m_center.SetX(std::stod(value));
		return true;
	case ParamKey::CenterY:
		m_center.SetY(std::stod(value));
		return true;
	case ParamKey::CenterZ:
		m_center.SetZ(std::stod(value));
		return true;
	default:
		return false;
	}
}

bool cadutils::SphereObject::GetParameters(std::vector<ParameterItem>& params)
{
	params.emplace_back(ParamKey::Radius, "Radius", std::to_string(m_radius), true);
	params.emplace_back(ParamKey::CenterX, "CenterX", std::to_string(m_center.GetX()), false);
	params.emplace_back(ParamKey::CenterY, "CenterY", std::to_string(m_center.GetY()), false);
	params.emplace_back(ParamKey::CenterZ, "CenterZ", std::to_string(m_center.GetZ()), false);
	return true;
}
