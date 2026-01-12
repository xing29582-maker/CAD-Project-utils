#pragma once

#include "DataExport.h"
#include "IObject.h"

namespace cadutils 
{        

    class CADUTILS_DATA_API Object : public IObject
    {
    public:
        explicit Object(const std::string &name);
        virtual ~Object() noexcept = default;

        virtual const std::string& GetObjectName() const override;
        virtual int64_t GetObjectId() const override;
        virtual TopoDS_Shape  buildShape() override;

    private:
        friend class Document;
        std::string m_objName;
        int64_t m_ObjId;
    };
} // namespace cadutils
