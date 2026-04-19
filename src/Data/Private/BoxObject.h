#pragma once

#include "Object.h"
#include "Point3d.h"

namespace cadutils
{
    class BoxObject : public Object
    {
        CAD_OBJECT_BEGIN(BoxObject);
            CAD_PROP(Point3d, center, DirtyFlags::Geometry)
            CAD_PROP(double, length, DirtyFlags::Geometry)
            CAD_PROP(double, width,  DirtyFlags::Geometry)
            CAD_PROP(double, height, DirtyFlags::Geometry)
        CAD_OBJECT_END;
    public:
        explicit BoxObject(const std::string& name, const Point3d& center,
                           double length, double width, double height);
        virtual ~BoxObject() noexcept = default;
        virtual std::shared_ptr<IBody> buildShape() override;
        virtual bool SetParameters(ParamKey key, std::string value) override;
        virtual bool GetParameters(std::vector<ParameterItem>& params) override;
    };
} // namespace cadutils