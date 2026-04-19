#include "RenderSystem.h"
#include "Document.h"
#include "GraphicsScene.h"
#include "GeometryData.h"
#include "IGraphicsNode.h"
#include "IRenderView.h"

#include <unordered_set>

using namespace cadutils;

cadutils::RenderSystem::RenderSystem(std::shared_ptr<GraphicsScene> gscene, const MeshGenerator& mesher, std::shared_ptr<IRenderView> renderView)
    : m_gscene(gscene)
    , m__mesher(mesher)
    , m_renderView(renderView)
{
}

void cadutils::RenderSystem::SyncFromDocument(const std::shared_ptr<Document>& doc, const TessellationOptions& opt, bool isAllBuild)
{
    if (isAllBuild)
    {
        for (const auto& obj : doc->GetObjects())
        {
            const auto id = obj->GetObjectId();
            std::shared_ptr<IGraphicsNode> gnode = m_gscene->GetOrCreate(id);
            std::shared_ptr<IBody> body = obj->buildShape();
            GeometryData geoData = m__mesher.Tessellate(body, opt);
            gnode->SetGeometryData(geoData);
        }
    }
    else
    {
        for (const auto& objId : doc->ConsumeDirty())
        {
            std::shared_ptr<IObject> obj = doc->GetobjectById(objId.id);
            if (!obj)
            {
                // Object was removed — remove from graphics scene
                m_gscene->Remove(objId.id);
                m_gscene->MarkDirty(objId.id);
                continue;
            }
            std::shared_ptr<IGraphicsNode> gnode = m_gscene->GetOrCreate(objId.id);
            std::shared_ptr<IBody> body = obj->buildShape();
            GeometryData geoData = m__mesher.Tessellate(body, opt);
            gnode->SetGeometryData(geoData);
            m_gscene->MarkDirty(objId.id);
        }
    }
}

void cadutils::RenderSystem::FullSyncFromDocument(const std::shared_ptr<Document>& doc, const TessellationOptions& opt)
{
    // Collect current document object IDs
    std::unordered_set<ObjectId> docIds;
    for (const auto& obj : doc->GetObjects())
    {
        docIds.insert(obj->GetObjectId());
    }

    // Remove graphics nodes for objects no longer in document
    auto allNodes = m_gscene->GetAllNodesWithId();
    for (const auto& [id, node] : allNodes)
    {
        if (docIds.find(id) == docIds.end())
        {
            m_gscene->Remove(id);
        }
    }

    // Add/update all document objects
    for (const auto& obj : doc->GetObjects())
    {
        const auto id = obj->GetObjectId();
        std::shared_ptr<IGraphicsNode> gnode = m_gscene->GetOrCreate(id);
        std::shared_ptr<IBody> body = obj->buildShape();
        GeometryData geoData = m__mesher.Tessellate(body, opt);
        gnode->SetGeometryData(geoData);
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
        m_renderView->refresh(m_gscene->GetAllNodesWithId(), true);
    }
    else
    {
        m_renderView->refresh(m_gscene->ConsumeDirtyIds(), false);
    }
}

std::shared_ptr<IRenderView> cadutils::RenderSystem::GetRenderView() const
{
    return m_renderView;
}
