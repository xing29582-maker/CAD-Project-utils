#pragma once

#include "IObject.h"
#include "PropertyRegistry.h"
#include "Property.h"
#include "ParameterItem.h"
#include "Document.h"

namespace cadutils 
{        

    class Object : public IObject
    {
        CAD_OBJECT_BEGIN(Object);
            CAD_PROP(std::string, objName, DirtyFlags::None)
            CAD_PROP(ObjectId, objId,  DirtyFlags::None)
            CAD_PROP(std::shared_ptr<IBody>, shapeBody, DirtyFlags::Geometry)
        CAD_OBJECT_END;
    public:
        explicit Object(const std::string &name);
        virtual ~Object() noexcept = default;

        virtual const std::string& GetObjectName() const override;
        virtual ObjectId GetObjectId() const override;
        virtual std::shared_ptr<IBody> buildShape() override;
        virtual bool SetParameters(ParamKey key, std::string value) override;
        virtual bool GetParameters(std::vector<ParameterItem>& params) override;
    protected:
        void SetShapeBpdy(const std::shared_ptr<IBody>& shape);
        std::shared_ptr<const IBody> GetShapeBody() const;
    private:
        friend class Document;
    };
} // namespace cadutils
