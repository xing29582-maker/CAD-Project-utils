#include "RenderView.h"
#include "OsgBackend.h"

using namespace cadutils;

RenderView::RenderView()
{
    m_widget = std::make_shared<OsgQtWidget>();

    // 只有 Render 模块知道 root，APP 永远不知道
    m_root = new osg::Group;
    m_widget->root()->addChild(m_root.get());
}

void cadutils::RenderView::refresh(const std::unordered_map<ObjectId, std::shared_ptr<IGraphicsNode>>& gRepNodes)
{
    //if (!m_doc) 
    //    return;
    OsgBackend backend;
    // 1) 遍历 GraphicsScene 中的所有节点
    for (const auto& node : gRepNodes)
    {
        const ObjectId id = node.first;  // ObjectId
        const std::shared_ptr<IGraphicsNode > gnode = node.second;  // GraphicsNode

        // 2) 如果该 ObjectId 在 OSG 中没有对应的 Node，创建一个
        if (m_nodes.find(id) == m_nodes.end())
        {
            osg::ref_ptr<osg::Node> osgNode = backend.BuildNode(gnode);
            m_root->addChild(osgNode);
            m_nodes[id] = osgNode;
        }
        // 3) 如果存在并且 GraphicsNode 被标记为脏，则更新节点
        else
        {
            osg::ref_ptr<osg::Node> osgNode = m_nodes[id];
            osgNode = backend.BuildNode(gnode);
        }
    }
}
