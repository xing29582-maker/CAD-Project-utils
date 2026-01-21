#include "Object.h"

using namespace cadutils;

cadutils::Object::Object(const std::string& name)
	:m_objName(name)
{
}

const std::string& cadutils::Object::GetObjectName() const
{
	return m_objName;
}

ObjectId cadutils::Object::GetObjectId() const
{
	return m_ObjId;
}

std::shared_ptr<IBody> cadutils::Object::buildShape()
{
	return nullptr;
}
