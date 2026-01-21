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
            const MeshGenerator &mesher , std::shared_ptr<IRenderView> renderView);

        // 每次 doc 变化后调用，或每帧调用（先简单每帧）
        void SyncFromDocument(const std::shared_ptr<cadutils::Document>& doc,
            const TessellationOptions& opt);

        std::vector<std::shared_ptr<IGraphicsNode>> GetAllGrepNodes() const;

        void Refresh();
    private:
        std::shared_ptr<GraphicsScene> m_gscene;
        MeshGenerator m__mesher;
        std::shared_ptr<IRenderView> m_renderView;
    };

} // namespace cadutils::scene