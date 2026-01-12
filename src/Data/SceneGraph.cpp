#include "SceneGraph.h"
#include "OccToOsg.h"
#include "Document.h"

// 你自己Document/Object接口按你现有的改

using namespace cadutils;

SceneGraph::SceneGraph()
{
    _root = new osg::Group();
}

void SceneGraph::syncFromDocument(const std::weak_ptr<cadutils::Document> doc)
{
    std::shared_ptr<cadutils::Document> docTempPtr = doc.lock();
    if (docTempPtr == nullptr)
    {
        return;
    }
    // 伪代码：遍历doc的objects
    for (auto objIter : docTempPtr->GetObjects())
    {
        const uint64_t id = objIter.second->GetObjectId();            // 你已有唯一ID更好

        auto* geoObj = dynamic_cast<IObject*>(objIter.second.get());
        if (!geoObj) continue;

        auto& item = _items[id];
        if (!item.node.valid())
        {
            // rebuild
            TessellationOptions opt;
            item.node = BuildOsgNodeFromShape(geoObj->buildShape(), opt);

            // 挂到root：简单做法先删旧的再加
            if (item.node.valid())
            {
                // 简化：你可以记录 child index；或者用 Switch/MatrixTransform 管理
                _root->addChild(item.node.get());
            }
        }
    }
}
