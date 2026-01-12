#pragma once

#include "GeometryExport.h"

#include <memory>

namespace cadutils
{
	class IBody;

	class CADUTILS_GEOMETRY_API GeoBuildUtils
	{
	public:
		std::shared_ptr<IBody> CreateSphere();
	};
}