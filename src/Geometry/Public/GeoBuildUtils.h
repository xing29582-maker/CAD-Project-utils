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
		static std::shared_ptr<IBody> CreateSphere(const Point3d& center ,double radius);
	};
}