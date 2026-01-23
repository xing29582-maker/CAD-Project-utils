#include "RenderView.h"
#include "OsgBackend.h"

#include <osg/Material>

using namespace cadutils;

RenderView::RenderView()
{
    m_widget = std::make_shared<OsgQtWidget>();

    // 只有 Render 模块知道 root，APP 永远不知道
    m_root = new osg::Group;
    m_widget->root()->addChild(m_root.get());
    m_widget->SetUiPickCallback([&](ObjectId id)
        {
            if (id == 0) return;

            SetSelected(id);   // Render 高亮
            if (m_pickCallBack)
                m_pickCallBack(id); // 通知 MainWindow 更新属性面板 
        });
}

void cadutils::RenderView::refresh(const std::unordered_map<ObjectId, std::shared_ptr<IGraphicsNode>>& gRepNodes)
{
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
            osgNode->setUserValue("ObjectId", static_cast<unsigned int>(id));
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

void cadutils::RenderView::SetOnPicked(PickCallback cb)
{
    m_pickCallBack = cb;
}

void cadutils::RenderView::SetSelected(ObjectId id)
{
    if (m_selected == id) return;

    // 取消旧高亮
    if (auto it = m_nodes.find(m_selected); it != m_nodes.end())
        ApplySelectedState(it->second.get(), false);

    m_selected = id;

    // 新高亮
    if (auto it = m_nodes.find(m_selected); it != m_nodes.end())
        ApplySelectedState(it->second.get(), true);
}

void cadutils::RenderView::ApplySelectedState(osg::Node* node, bool selected)
{
    osg::StateSet* ss = node->getOrCreateStateSet();

    osg::ref_ptr<osg::Material> mat = new osg::Material();
    if (selected)
        mat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.f, 0.8f, 0.2f, 1.f));
    else
        mat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0.8f, 0.8f, 0.8f, 1.f));

    ss->setAttributeAndModes(mat.get(), osg::StateAttribute::ON);
}
