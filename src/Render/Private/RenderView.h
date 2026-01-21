#pragma once

#include "IRenderView.h"
#include "NameDefine.h"
#include "OsgQtWidget.h"
#include <osg/Group>

namespace cadutils
{

    class RenderView final : public IRenderView
    {
    public:
        RenderView();

        QWidget* widget() override { return m_widget.get(); }

        void setDocument(const Document* doc) override
        {
            m_doc = doc;
        }

        virtual void refresh(const std::unordered_map<ObjectId, std::shared_ptr<IGraphicsNode>>& gRepNodes) override;

    private:
        const Document* m_doc = nullptr;
        std::shared_ptr<OsgQtWidget> m_widget;
        osg::ref_ptr<osg::Group> m_root;
        std::unordered_map<ObjectId, osg::ref_ptr<osg::Node>> m_nodes; // objectId -> render node
    };
}