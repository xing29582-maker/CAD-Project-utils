#include "SphereObject.h"
#include <BRepPrimAPI_MakeSphere.hxx>

using namespace cadutils;

cadutils::SphereObject::SphereObject(const std::string& name, double radius, gp_Pnt center)
	:Object(name)
	, m_radius(radius)
	, m_center(center)
{
}

TopoDS_Shape cadutils::SphereObject::buildShape()
{
	return BRepPrimAPI_MakeSphere(m_center, m_radius).Shape();
}
