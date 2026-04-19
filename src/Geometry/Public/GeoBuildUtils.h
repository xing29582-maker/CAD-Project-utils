#pragma once

#include "GeometryExport.h"
#include "Point3d.h"

#include <memory>

namespace cadutils
{
	class IBody;

	class CADUTILS_GEOMETRY_API GeoBuildUtils
	{
	public:
		static std::shared_ptr<IBody> CreateSphere(const Point3d& center, double radius);
		static std::shared_ptr<IBody> CreateBox(const Point3d& center, double length, double width, double height);
		static std::shared_ptr<IBody> CreateCylinder(const Point3d& center, double radius, double height);
	};
}