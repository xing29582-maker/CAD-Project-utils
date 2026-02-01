#include "SphereObject.h"
#include "GeoBuildUtils.h"

using namespace cadutils;

cadutils::SphereObject::SphereObject(const std::string& name, const Point3d& center, double radius)
	:Object(name)
{
	m_radius.set(radius);
	m_center.set(center);
}

std::shared_ptr<IBody> cadutils::SphereObject::buildShape()
{
	return GeoBuildUtils::CreateSphere(m_center.get(), m_radius.get());
}

const Point3d& cadutils::SphereObject::GetCenter() const
{
	return m_center.get();
}

double cadutils::SphereObject::GetRadius() const
{
	return m_radius.get();
}

void cadutils::SphereObject::SetRadius(double radius)
{
	m_radius.set(radius);
}

bool cadutils::SphereObject::SetParameters(ParamKey key, std::string value)
{
	switch (key)
	{
	case ParamKey::Radius:
		m_radius.set(std::stod(value));
		return true;
	default:
		return false;
	}
}

bool cadutils::SphereObject::GetParameters(std::vector<ParameterItem>& params)
{
	params.emplace_back(ParamKey::Radius, "Radius", std::to_string(m_radius.get()), true);
	return true;
}
