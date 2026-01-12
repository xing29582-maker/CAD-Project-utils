#include "Object.h"
#include <BRepPrimAPI_MakeSphere.hxx>

using namespace cadutils;

cadutils::Object::Object(const std::string& name)
	:m_objName(name)
{
}

const std::string& cadutils::Object::GetObjectName() const
{
	return m_objName;
}

int64_t cadutils::Object::GetObjectId() const
{
	return m_ObjId;
}

TopoDS_Shape cadutils::Object::buildShape()
{
	return TopoDS_Shape();
}
