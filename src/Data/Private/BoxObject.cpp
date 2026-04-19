#include "BoxObject.h"
#include "GeoBuildUtils.h"

using namespace cadutils;

CAD_DEFAULT_CTOR(BoxObject)
{

}

BoxObject::BoxObject(const std::string& name, const Point3d& center,
                     double length, double width, double height)
    : Object(name)
{
    m_center.set(center);
    m_length.set(length);
    m_width.set(width);
    m_height.set(height);
}

std::shared_ptr<IBody> BoxObject::buildShape()
{
    return GeoBuildUtils::CreateBox(m_center.get(), m_length.get(), m_width.get(), m_height.get());
}

bool BoxObject::SetParameters(ParamKey key, std::string value)
{
    switch (key)
    {
    case ParamKey::Length:
        m_length.set(std::stod(value));
        return true;
    case ParamKey::Width:
        m_width.set(std::stod(value));
        return true;
    case ParamKey::Height:
        m_height.set(std::stod(value));
        return true;
    default:
        return Object::SetParameters(key, value);
    }
}

bool BoxObject::GetParameters(std::vector<ParameterItem>& params)
{
    Object::GetParameters(params);
    params.emplace_back(ParamKey::Length, "Length", std::to_string(m_length.get()), true);
    params.emplace_back(ParamKey::Width,  "Width",  std::to_string(m_width.get()),  true);
    params.emplace_back(ParamKey::Height, "Height", std::to_string(m_height.get()), true);
    return true;
}