#pragma once

#include "DataExport.h"
#include "Point3d.h"

#include <string>
#include <memory>

namespace cadutils
{
	class IObject;
	class CADUTILS_DATA_API ObjectFactory
	{
		public:
			static std::shared_ptr<IObject> CreateSphereObject(const std::string& name, const Point3d& center, double radius);
			static std::shared_ptr<IObject> CreateBoxObject(const std::string& name, const Point3d& center, double length, double width, double height);
			static std::shared_ptr<IObject> CreateCylinderObject(const std::string& name, const Point3d& center, double radius, double height);
	};
}