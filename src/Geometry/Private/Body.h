#pragma once

#include "IBody.h"

#include <TopoDS_Solid.hxx>

namespace cadutils
{
	class Body : public IBody
	{
	public:
		explicit Body(const TopoDS_Solid& shape);

	private:
		TopoDS_Solid m_shape;
	};
}