#include "OccBody.h"


using namespace cadutils;

cadutils::OccBody::OccBody(const TopoDS_Solid& shape)
	:m_shape(shape)
{
}

const TopoDS_Shape& cadutils::OccBody::GetOccShape() const
{
	return m_shape;
}
