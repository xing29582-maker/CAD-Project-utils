#pragma once

#include <string>

class TopoDS_Shape;

namespace cadutils 
{        
    class IObject 
    {
    public:
        virtual ~IObject() = default;

        virtual const std::string& GetObjectName() const = 0;
        virtual int64_t GetObjectId() const = 0;
        virtual TopoDS_Shape  buildShape() = 0;
    };
} // namespace cadutils
