#pragma once
#include "RenderExport.h"
#include "NameDefine.h"

#include <unordered_map>

class QWidget;

namespace cadutils
{
    class Document;
    class IGraphicsNode;

    class IRenderView {
    public:
        virtual ~IRenderView() = default;

        virtual QWidget* widget() = 0;
        // APP 只说“内容变了”，具体怎么 sync 由 Render 决定
        virtual void setDocument(const Document* doc) = 0;
        virtual void refresh(const std::unordered_map<ObjectId, std::shared_ptr<IGraphicsNode>>& gRepNodes) = 0;

        CADUTILS_RENDER_API static std::shared_ptr<IRenderView> createRenderView();
    };

}