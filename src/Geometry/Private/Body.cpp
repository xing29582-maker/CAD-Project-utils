#include "Body.h"


using namespace cadutils;

cadutils::Body::Body(const TopoDS_Solid& shape)
	:m_shape(shape)
{
}
