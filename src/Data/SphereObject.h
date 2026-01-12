#pragma once

#include "Object.h"
#include <gp_Pnt.hxx>

namespace cadutils 
{        

    class CADUTILS_DATA_API SphereObject : public Object
    {
    public:
        explicit SphereObject(const std::string &name , double radius, gp_Pnt center );
        virtual ~SphereObject() noexcept = default;

        TopoDS_Shape  buildShape() override;

    private:
        gp_Pnt m_center;
        double m_radius;
    };
} // namespace cadutils
