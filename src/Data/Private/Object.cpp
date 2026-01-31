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

bool cadutils::Object::SetParameters(ParamKey key, std::string value)
{
    switch (key)
    {
    case ParamKey::Name:
        m_objName = value;
        return true;
    default:
        return false;
    }
}

bool cadutils::Object::GetParameters(std::vector<ParameterItem>& params)
{ 
    params.emplace_back(ParamKey::Id, "Id" , std::to_string(m_ObjId), false);
    params.emplace_back(ParamKey::Name, "Name", m_objName, false);
    return true;
}
