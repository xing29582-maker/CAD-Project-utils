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
