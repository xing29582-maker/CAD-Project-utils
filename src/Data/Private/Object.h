#pragma once

#include "IObject.h"

namespace cadutils 
{        

    class Object : public IObject
    {
    public:
        explicit Object(const std::string &name);
        virtual ~Object() noexcept = default;

        virtual const std::string& GetObjectName() const override;
        virtual ObjectId GetObjectId() const override;
        virtual std::shared_ptr<IBody> buildShape() override;
        virtual bool SetParameters(ParamKey key, std::string value) override;
        virtual bool GetParameters(std::vector<ParameterItem>& params) override;
    protected:
        std::shared_ptr<IBody> m_shapeBody;
    private:
        friend class Document;
        std::string m_objName;
        ObjectId m_ObjId;
    };
} // namespace cadutils
