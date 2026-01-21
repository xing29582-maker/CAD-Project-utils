#include "GraphicsNode.h"

using namespace cadutils;

GraphicsNode::GraphicsNode(ObjectId id)
	:m_ownerObjId(id)
{
	
}

GraphicsNode::~GraphicsNode()
{
}

void GraphicsNode::SetGeometryData(const GeometryData &geoData)
{
	m_geoData = geoData;
}

GeometryData GraphicsNode::GetGeometryData() const
{
	return m_geoData;
}


