#pragma once

#include "IGraphicsNode.h"


namespace cadutils
{
    class GraphicsNode : public IGraphicsNode
    {
    public:
        GraphicsNode(ObjectId id);
        virtual ~GraphicsNode();

        virtual void SetGeometryData(const GeometryData &geoData) override;
        virtual GeometryData GetGeometryData() const override;

    private:
        ObjectId m_ownerObjId;
        GeometryData m_geoData;
    };
}
