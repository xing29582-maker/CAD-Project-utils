#include "RenderView.h"
#include "OsgBackend.h"

#include <osg/Material>

using namespace cadutils;

RenderView::RenderView()
    :m_selected(0)
{
    m_widget = std::make_shared<OsgQtWidget>();

    // ֻ�� Render ģ��֪�� root��APP ��Զ��֪��
    m_root = new osg::Group;
    m_widget->root()->addChild(m_root.get());
    m_widget->SetUiPickCallback([&](ObjectId id)
        {
            SetSelected(id);
            if (m_pickCallBack)
                m_pickCallBack(id);
        });
}

void cadutils::RenderView::refresh(const std::unordered_map<ObjectId, std::shared_ptr<IGraphicsNode>>& gRepNodes, bool fullSync)
{
    if (gRepNodes.empty() && !fullSync)
        return;

    OsgBackend backend;
    bool hasNewNodes = false;

    // Cleanup phase
    std::vector<ObjectId> toRemove;
    if (fullSync)
    {
        // Full sync: remove containers not present in the new full map
        for (const auto& [id, container] : m_containers)
        {
            if (gRepNodes.find(id) == gRepNodes.end())
                toRemove.push_back(id);
        }
    }
    // Also remove containers whose gnode is explicitly null (deleted objects in dirty map)
    for (const auto& [id, gnode] : gRepNodes)
    {
        if (!gnode && m_containers.find(id) != m_containers.end())
            toRemove.push_back(id);
    }
    for (ObjectId id : toRemove)
    {
        auto it = m_containers.find(id);
        if (it != m_containers.end())
        {
            m_root->removeChild(it->second.get());
            m_containers.erase(it);
        }
    }

    for (const auto& node : gRepNodes)
    {
        const ObjectId id = node.first;
        const std::shared_ptr<IGraphicsNode> gnode = node.second;

        if (!gnode) continue;  // Safety: skip null nodes

        osg::ref_ptr<osg::MatrixTransform> container;
        auto containerIter = m_containers.find(id);
        if (containerIter == m_containers.end())
        {
            container = new osg::MatrixTransform();
            container->setUserValue("ObjectId", static_cast<unsigned int>(id));
            m_root->addChild(container);
            m_containers[id] = container;
            hasNewNodes = true;
        }
        else
        {
            container = containerIter->second;
        }
        container->removeChildren(0, container->getNumChildren());
        osg::ref_ptr<osg::Node> osgNode = backend.BuildNode(gnode);
        if (osgNode.valid())
            container->addChild(osgNode.get());
    }

    // Auto-fit camera when new nodes are added or during full sync
    if ((hasNewNodes || fullSync) && m_widget && m_widget->viewer())
    {
        m_widget->viewer()->home();
    }
}

void cadutils::RenderView::SetOnPicked(PickCallback cb)
{
    m_pickCallBack = cb;
}

void cadutils::RenderView::SetSelected(ObjectId id)
{
    if (m_selected == id) return;

    // ȡ���ɸ���
    if (auto it = m_containers.find(m_selected); it != m_containers.end())
        ApplySelectedState(it->second.get(), false);

    m_selected = id;

    // �¸���
    if (auto it = m_containers.find(m_selected); it != m_containers.end())
        ApplySelectedState(it->second.get(), true);
}

void cadutils::RenderView::ApplySelectedState(osg::MatrixTransform* node, bool selected)
{
    osg::StateSet* ss = node->getOrCreateStateSet();

    osg::ref_ptr<osg::Material> mat = new osg::Material();
    if (selected)
        mat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.f, 0.8f, 0.2f, 1.f));
    else
        mat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0.8f, 0.8f, 0.8f, 1.f));

    ss->setAttributeAndModes(mat.get(), osg::StateAttribute::ON);
}
