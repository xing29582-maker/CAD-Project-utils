#include "IGraphicsNode.h"
#include "GraphicsNode.h"

using namespace cadutils;

std::shared_ptr<IGraphicsNode> cadutils::IGraphicsNode::CreateGraphicsNode(ObjectId id)
{
	return std::make_shared<GraphicsNode>(id);
}
