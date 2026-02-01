#pragma once

#include "DataExport.h"
#include "NameDefine.h"
#include "Point3d.h"
#include "ParameterItem.h"
#include <string>
#include <vector>

namespace cadutils 
{        
    class IBody;
    class Document;

    class IObject 
    {
    public:
        virtual ~IObject() = default;

        virtual const std::string& GetObjectName() const = 0;
        virtual ObjectId GetObjectId() const = 0;
        virtual std::shared_ptr<IBody> buildShape() = 0;
        virtual bool SetParameters(ParamKey key,std::string value) = 0;
        virtual bool GetParameters(std::vector<ParameterItem>& params) = 0;
        virtual void OnAddedToDocument(Document &doc) = 0;
    };
} // namespace cadutils
