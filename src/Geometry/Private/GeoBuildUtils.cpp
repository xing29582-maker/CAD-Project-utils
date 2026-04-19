#include "GeoBuildUtils.h"
#include "OccBody.h"

#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <gp_Ax2.hxx>
#include <TopoDS_Solid.hxx>

using namespace cadutils;


std::shared_ptr<IBody> cadutils::GeoBuildUtils::CreateSphere(const Point3d& center, double radius)
{
    gp_Pnt centerPnt(center.GetX(), center.GetY(), center.GetZ());
    TopoDS_Solid bodyShape = BRepPrimAPI_MakeSphere(centerPnt, radius).Solid();
    return std::make_shared<OccBody>(bodyShape);
}

std::shared_ptr<IBody> cadutils::GeoBuildUtils::CreateBox(const Point3d& center, double length, double width, double height)
{
    // Box corner = center - half extents
    gp_Pnt corner(
        center.GetX() - length * 0.5,
        center.GetY() - width * 0.5,
        center.GetZ() - height * 0.5);
    TopoDS_Solid bodyShape = BRepPrimAPI_MakeBox(corner, length, width, height).Solid();
    return std::make_shared<OccBody>(bodyShape);
}

std::shared_ptr<IBody> cadutils::GeoBuildUtils::CreateCylinder(const Point3d& center, double radius, double height)
{
    // Cylinder axis at center, pointing +Z, bottom at center.z - height/2
    gp_Pnt bottomCenter(center.GetX(), center.GetY(), center.GetZ() - height * 0.5);
    gp_Ax2 axis(bottomCenter, gp_Dir(0, 0, 1));
    TopoDS_Solid bodyShape = BRepPrimAPI_MakeCylinder(axis, radius, height).Solid();
    return std::make_shared<OccBody>(bodyShape);
}
