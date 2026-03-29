#include "Object.h"

using namespace cadutils;

CAD_DEFAULT_CTOR(Object)
{

}

cadutils::Object::Object(const std::string& name)
{
    m_objName.set(name);
}

const std::string& cadutils::Object::GetObjectName() const
{
	return m_objName.get();
}

ObjectId cadutils::Object::GetObjectId() const
{
	return m_objId.get();
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
        m_objName.set(value);
        return true;
    default:
        return false;
    }
}

bool cadutils::Object::GetParameters(std::vector<ParameterItem>& params)
{ 
    params.emplace_back(ParamKey::Id, "Id" , std::to_string(m_objId.get()), false);
    params.emplace_back(ParamKey::Name, "Name", m_objName.get(), false);
    return true;
}

void cadutils::Object::OnPropertyChanging(PropertyBase& prop)
{
    if (!m_ownerDoc)
        return;

    m_ownerDoc->OnPropertyChanging(*this, prop);
}

void cadutils::Object::OnPropertyChanged(PropertyBase& prop)
{
    if (!m_ownerDoc)
        return;

    m_ownerDoc->OnPropertyChanged(*this, prop);
}

Document* cadutils::Object::GetOwnerDoc() const noexcept
{
    return m_ownerDoc;
}

void cadutils::Object::SetShapeBpdy(const std::shared_ptr<IBody>& shape)
{
    m_shapeBody.set(shape);
}

std::shared_ptr<const IBody> cadutils::Object::GetShapeBody() const
{
    return m_shapeBody.get();
}

void cadutils::Object::SetOwnerDoc(Document *doc) noexcept
{
    m_ownerDoc = doc;
}
