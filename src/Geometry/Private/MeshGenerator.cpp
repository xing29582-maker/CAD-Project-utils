#include "MeshGenerator.h"

#include "OccBody.h" 
#include "GeometryData.h"

#include <BRepMesh_IncrementalMesh.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopAbs_Orientation.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <Poly_Triangle.hxx>
#include <TopLoc_Location.hxx>
#include <gp_Trsf.hxx>
#include <gp_Pnt.hxx>


using namespace cadutils;
using namespace std;

MeshGenerator::MeshGenerator()
{
}

GeometryData cadutils::MeshGenerator::Tessellate(const std::shared_ptr<IBody> body, const TessellationOptions& opt)
{
    GeometryData geoData;
    if (!body)
        return geoData;
    // 1) 拿到 OCC shape（只在 geometry/private 做）
    std::shared_ptr<OccBody> occBody = std::dynamic_pointer_cast<OccBody>(body);
    if (!occBody)
        return geoData;

    const TopoDS_Shape& shape = occBody->GetOccShape();

    // 2) 触发 OCC 三角化
    BRepMesh_IncrementalMesh mesher(shape, opt.deflection);
    mesher.Perform();

    // 简化：不做顶点去重：每个三角形写 3 个点
    // 这样 indices 就可以连续递增
    uint32_t baseIndex = 0;

    TopExp_Explorer ex;
    for (ex.Init(shape, TopAbs_FACE); ex.More(); ex.Next())
    {
        const TopoDS_Face face = TopoDS::Face(ex.Current());

        TopLoc_Location loc;
        Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, loc);
        if (tri.IsNull()) continue;

        const gp_Trsf trsf = loc.Transformation();
        const bool reversed = (face.Orientation() == TopAbs_REVERSED);

        const int nbTris = tri->NbTriangles();
        for (int t = 1; t <= nbTris; ++t)
        {
            Poly_Triangle tr = tri->Triangle(t);

            int n1, n2, n3;
            tr.Get(n1, n2, n3);
            if (reversed) std::swap(n2, n3);

            const gp_Pnt p1 = tri->Node(n1).Transformed(trsf);
            const gp_Pnt p2 = tri->Node(n2).Transformed(trsf);
            const gp_Pnt p3 = tri->Node(n3).Transformed(trsf);

            geoData.positions.push_back(Point3d(p1.X(), p1.Y(), p1.Z()));
            geoData.positions.push_back(Point3d(p2.X(), p2.Y(), p2.Z()));
            geoData.positions.push_back(Point3d(p3.X(), p3.Y(), p3.Z()));

            geoData.indices.push_back(baseIndex + 0);
            geoData.indices.push_back(baseIndex + 1);
            geoData.indices.push_back(baseIndex + 2);
            baseIndex += 3;

            if (opt.computeNormals)
            {
                // 简单三角形法线（未归一化也行，最好归一化）
                const Point3d a = geoData.positions[geoData.positions.size() - 3];
                const Point3d b = geoData.positions[geoData.positions.size() - 2];
                const Point3d c = geoData.positions[geoData.positions.size() - 1];

                Vector3d a2b = b - a;
                Vector3d a2c = c - a;
                Vector3d normal = a2b ^ a2c;
                normal.Normalize();

                geoData.normals.push_back(normal);
                geoData.normals.push_back(normal);
                geoData.normals.push_back(normal);
            }
        }
    }
    return geoData;
}
