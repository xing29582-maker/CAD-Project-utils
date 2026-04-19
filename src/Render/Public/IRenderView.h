#pragma once
#include "RenderExport.h"
#include "NameDefine.h"

#include <unordered_map>
#include <functional>

class QWidget;

namespace cadutils
{
    class Document;
    class IGraphicsNode;

    using PickCallback = std::function<void(ObjectId)>;
    class IRenderView 
    {
    public:
        virtual ~IRenderView() = default;
        virtual QWidget* widget() = 0;
        // APP ֻ˵�����ݱ��ˡ���������ô sync �� Render ����
        virtual void refresh(const std::unordered_map<ObjectId, std::shared_ptr<IGraphicsNode>>& gRepNodes, bool fullSync = false) = 0;
        virtual void SetOnPicked(PickCallback cb) = 0;
        virtual void SetSelected(ObjectId id) = 0;
        CADUTILS_RENDER_API static std::shared_ptr<IRenderView> createRenderView();
    };

}