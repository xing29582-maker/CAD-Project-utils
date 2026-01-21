#pragma once

#include "GeometryExport.h"
#include "TessellationOptions.h"
#include "GeometryData.h"

#include <memory>

namespace cadutils
{
	class IBody;

	class CADUTILS_GEOMETRY_API MeshGenerator
	{
	public:
		explicit MeshGenerator();
		GeometryData Tessellate(const std::shared_ptr<IBody> body, const TessellationOptions& opt);
	private:
	};
}