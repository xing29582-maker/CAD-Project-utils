#pragma once

#include "Point3d.h"
#include "Vector3d.h"

#include<vector>

namespace cadutils
{
    struct GeometryData 
    {
        std::vector<Point3d> positions;
        std::vector<Vector3d> normals;
        std::vector<uint32_t> indices;
        // optional: edges, bbox, version
    };
}
