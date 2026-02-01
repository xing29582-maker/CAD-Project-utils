#pragma once

#include "Object.h"
#include "Point3d.h"

namespace cadutils 
{        
    class SphereObject : public Object
    {
        CAD_OBJECT_BEGIN(SphereObject);
            CAD_PROP(Point3d, center,  DirtyFlags::Geometry)
            CAD_PROP(double, radius,  DirtyFlags::Geometry)
        CAD_OBJECT_END;
    public:
        explicit SphereObject(const std::string &name , const Point3d& center , double radius);
        virtual ~SphereObject() noexcept = default;
        virtual std::shared_ptr<IBody> buildShape() override;

    public:
        const Point3d &GetCenter() const;
        double GetRadius() const;
        void SetRadius(double radius);
        virtual bool SetParameters(ParamKey key, std::string value) override;
        virtual bool GetParameters(std::vector<ParameterItem>& params) override;
    };
} // namespace cadutils
