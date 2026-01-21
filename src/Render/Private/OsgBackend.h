#pragma once

#include "GeometryData.h"
#include "IGraphicsNode.h"

#include <memory>

#include <osg/Node>

namespace cadutils
{
    class OsgBackend 
    {
    public:
        osg::ref_ptr<osg::Node> BuildNode(const std::shared_ptr<IGraphicsNode> node);
    };
}
