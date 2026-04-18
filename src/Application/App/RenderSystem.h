#pragma once

#include <memory>
#include <vector>

#include "MeshGenerator.h"

namespace cadutils
{

    struct TessellationOptions;
    class Document;
    class IBody;
    class GraphicsScene;
    class IGraphicsNode;
    class IRenderView;

    class RenderSystem
    {
    public:
        RenderSystem(std::shared_ptr<GraphicsScene> gscene,
            const MeshGenerator& mesher, std::shared_ptr<IRenderView> renderView);

        // Sync from document: rebuild geometry for dirty or all objects
        void SyncFromDocument(const std::shared_ptr<cadutils::Document>& doc,
            const TessellationOptions& opt, bool isAllBuild = true);

        // Full sync: handles add, remove, and modify
        void FullSyncFromDocument(const std::shared_ptr<cadutils::Document>& doc,
            const TessellationOptions& opt);

        std::vector<std::shared_ptr<IGraphicsNode>> GetAllGrepNodes() const;

        void Refresh(bool isAll = true);

        std::shared_ptr<IRenderView> GetRenderView() const;

    private:
        std::shared_ptr<GraphicsScene> m_gscene;
        MeshGenerator m__mesher;
        std::shared_ptr<IRenderView> m_renderView;
    };

} // namespace cadutils