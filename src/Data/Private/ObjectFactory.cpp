#include "ObjectFactory.h"
#include "SphereObject.h"


using namespace cadutils;

std::shared_ptr<IObject> cadutils::ObjectFactory::CreateSphereObject(const std::string& name, const Point3d& center, double radius)
{
    return std::make_shared<SphereObject>(name, center, radius);
}
