#include "ObjectFactory.h"
#include "SphereObject.h"
#include "BoxObject.h"
#include "CylinderObject.h"

using namespace cadutils;

std::shared_ptr<IObject> cadutils::ObjectFactory::CreateSphereObject(const std::string& name, const Point3d& center, double radius)
{
    return std::make_shared<SphereObject>(name, center, radius);
}

std::shared_ptr<IObject> cadutils::ObjectFactory::CreateBoxObject(const std::string& name, const Point3d& center, double length, double width, double height)
{
    return std::make_shared<BoxObject>(name, center, length, width, height);
}

std::shared_ptr<IObject> cadutils::ObjectFactory::CreateCylinderObject(const std::string& name, const Point3d& center, double radius, double height)
{
    return std::make_shared<CylinderObject>(name, center, radius, height);
}
