#pragma once

#include "IBody.h"
#include "IGeometry.h"

#include <TopoDS_Solid.hxx>

namespace cadutils
{
	class OccBody : public IGeometry, public IBody
	{
	public:
		explicit OccBody(const TopoDS_Solid& shape);

		const TopoDS_Shape& GetOccShape() const;
	private:
		TopoDS_Solid m_shape;
	};
}