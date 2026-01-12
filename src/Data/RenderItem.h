#pragma once
#include <osg/Node>
#include <cstdint>

struct RenderItem
{
    uint64_t objectId = 0;        // 你自己的唯一ID
    osg::ref_ptr<osg::Node> node; // 场景节点
};
