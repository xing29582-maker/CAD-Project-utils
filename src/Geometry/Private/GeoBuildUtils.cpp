#include "GeoBuildUtils.h"
#include "Body.h"

#include <BRepPrimAPI_MakeSphere.hxx>
#include <TopoDS_Solid.hxx>

using namespace cadutils;


std::shared_ptr<IBody> cadutils::GeoBuildUtils::CreateSphere(const Point3d& center, double radius)
{
    gp_Pnt centerPnt(center.GetX(), center.GetY(), center.GetZ());
    TopoDS_Solid bodyShape = BRepPrimAPI_MakeSphere(centerPnt, radius).Solid();
    return std::make_shared<Body>(bodyShape);
}
