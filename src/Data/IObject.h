#pragma once

#include <string>
#include <memory>

namespace cadutils 
{        
    class IBody;

    class IObject 
    {
    public:
        virtual ~IObject() = default;

        virtual const std::string& GetObjectName() const = 0;
        virtual int64_t GetObjectId() const = 0;
        virtual std::shared_ptr<IBody> buildShape() = 0;
    };
} // namespace cadutils
