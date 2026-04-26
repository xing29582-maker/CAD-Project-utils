#include "CylinderObject.h"
#include "GeoBuildUtils.h"

using namespace cadutils;

CAD_DEFAULT_CTOR(CylinderObject)
{
    // Initialize with default values for deserialization
    m_center.SetValueSilent(Point3d(0, 0, 0));
    m_radius.SetValueSilent(1.0);
    m_height.SetValueSilent(2.0);
}

CylinderObject::CylinderObject(const std::string& name, const Point3d& center,
                               double radius, double height)
    : Object(name)
{
    m_center.set(center);
    m_radius.set(radius);
    m_height.set(height);
}

std::shared_ptr<IBody> CylinderObject::buildShape()
{
    return GeoBuildUtils::CreateCylinder(m_center.get(), m_radius.get(), m_height.get());
}

bool CylinderObject::SetParameters(ParamKey key, std::string value)
{
    switch (key)
    {
    case ParamKey::Radius:
        m_radius.set(std::stod(value));
        return true;
    case ParamKey::Height:
        m_height.set(std::stod(value));
        return true;
    default:
        return Object::SetParameters(key, value);
    }
}

bool CylinderObject::GetParameters(std::vector<ParameterItem>& params)
{
    Object::GetParameters(params);
    params.emplace_back(ParamKey::Radius, "Radius", std::to_string(m_radius.get()), true);
    params.emplace_back(ParamKey::Height, "Height", std::to_string(m_height.get()), true);
    return true;
}