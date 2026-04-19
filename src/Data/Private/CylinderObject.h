#pragma once

#include "Object.h"
#include "Point3d.h"

namespace cadutils
{
    class CylinderObject : public Object
    {
        CAD_OBJECT_BEGIN(CylinderObject);
            CAD_PROP(Point3d, center, DirtyFlags::Geometry)
            CAD_PROP(double, radius, DirtyFlags::Geometry)
            CAD_PROP(double, height, DirtyFlags::Geometry)
        CAD_OBJECT_END;
    public:
        explicit CylinderObject(const std::string& name, const Point3d& center,
                                double radius, double height);
        virtual ~CylinderObject() noexcept = default;
        virtual std::shared_ptr<IBody> buildShape() override;
        virtual bool SetParameters(ParamKey key, std::string value) override;
        virtual bool GetParameters(std::vector<ParameterItem>& params) override;
    };
} // namespace cadutils