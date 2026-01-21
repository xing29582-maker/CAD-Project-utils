#pragma once

#include "RenderExport.h"
#include "GeometryData.h"
#include "NameDefine.h"

#include<memory>

namespace cadutils
{

    class IGraphicsNode 
    {
    public:
        virtual ~IGraphicsNode() = default;

        virtual void SetGeometryData(const GeometryData &geoData) = 0;
        virtual GeometryData GetGeometryData() const = 0;

        CADUTILS_RENDER_API static std::shared_ptr<IGraphicsNode> CreateGraphicsNode(ObjectId id);
    };
}


