#include "RenderSystem.h"
#include "Document.h"
#include "GraphicsScene.h"
#include "GeometryData.h"
#include "IGraphicsNode.h"
#include "IRenderView.h"

using namespace cadutils;

cadutils::RenderSystem::RenderSystem(std::shared_ptr<GraphicsScene> gscene, const MeshGenerator &mesher, std::shared_ptr<IRenderView> renderView)
    :m_gscene(gscene)
    ,m__mesher(mesher)
    ,m_renderView(renderView)
{
}

void cadutils::RenderSystem::SyncFromDocument(const std::shared_ptr<Document>& doc, const TessellationOptions& opt, bool isAllBuild)
{
    // 1) 遍历 Document 中的所有对象
    if (isAllBuild)
    {
        for (const auto& obj : doc->GetObjects())
        {
            const auto id = obj->GetObjectId();
            std::shared_ptr<IGraphicsNode> gnode = m_gscene->GetOrCreate(id);  // 取出或创建一个 GraphicsNode

            std::shared_ptr<IBody> body = obj->buildShape();  // 获取几何（IBody）
            GeometryData geoData = m__mesher.Tessellate(body, opt); // 生成 GeometryData

            gnode->SetGeometryData(geoData); // 塞入 GraphicsNode

        }
    }  
    else
    {
        for (const ObjectId& objId : doc->ConsumeDirtyIds())
        {
            std::shared_ptr<IGraphicsNode> gnode = m_gscene->GetOrCreate(objId);  // 取出或创建一个 GraphicsNode
            std::shared_ptr<IObject> obj = doc->GetobjectById(objId);
            std::shared_ptr<IBody> body = obj->buildShape();  // 获取几何（IBody）
            GeometryData geoData = m__mesher.Tessellate(body, opt); // 生成 GeometryData
            gnode->SetGeometryData(geoData); // 塞入 GraphicsNode
            m_gscene->MarkDirty(objId);

        }
    }
}

std::vector<std::shared_ptr<IGraphicsNode>> cadutils::RenderSystem::GetAllGrepNodes() const
{
    return m_gscene->GetItems();
}

void cadutils::RenderSystem::Refresh(bool isAll)
{
    if (isAll)
    {
        m_renderView->refresh(m_gscene->GetAllNodesWithId());
    }
    else
    {
        m_renderView->refresh(m_gscene->ConsumeDirtyIds());
    }
}

std::shared_ptr<IRenderView> cadutils::RenderSystem::GetRenderView() const
{
    return m_renderView;
}
