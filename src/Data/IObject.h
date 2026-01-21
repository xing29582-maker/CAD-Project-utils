#pragma once

#include "DataExport.h"
#include "NameDefine.h"
#include "Point3d.h"

#include <string>

namespace cadutils 
{        
    class IBody;

    class IObject 
    {
    public:
        virtual ~IObject() = default;

        virtual const std::string& GetObjectName() const = 0;
        virtual ObjectId GetObjectId() const = 0;
        virtual std::shared_ptr<IBody> buildShape() = 0;

        CADUTILS_DATA_API static std::shared_ptr<IObject> CreateSphereObject(const std::string& name, const Point3d& center, double radius);
    };
} // namespace cadutils
