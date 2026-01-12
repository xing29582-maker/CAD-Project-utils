#include "OccToOsg.h"

#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopLoc_Location.hxx>
#include <Poly_Triangulation.hxx>
#include <TopoDS_Face.hxx>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/PrimitiveSet>

static osg::ref_ptr<osg::Geometry> makeGeom()
{
    auto g = new osg::Geometry();
    g->setUseVertexBufferObjects(true);
    g->setUseDisplayList(false);
    return g;
}

osg::ref_ptr<osg::Node> BuildOsgNodeFromShape(const TopoDS_Shape& shape, const TessellationOptions& opt)
{
    // 1) tessellate
    BRepMesh_IncrementalMesh mesher(shape, opt.deflection);
    mesher.Perform();

    auto verts = new osg::Vec3Array();
    auto norms = opt.computeNormals ? new osg::Vec3Array() : nullptr;
    auto indices = new osg::DrawElementsUInt(GL_TRIANGLES);

    // 简化：不做顶点去重；按“每个三角形三个点”写入
    TopExp_Explorer ex;
    for (ex.Init(shape, TopAbs_FACE); ex.More(); ex.Next())
    {
        TopoDS_Face face = TopoDS::Face(ex.Current());

        TopLoc_Location loc;
        Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, loc);
        if (tri.IsNull()) continue;

        const gp_Trsf trsf = loc.Transformation();
        const bool reversed = (face.Orientation() == TopAbs_REVERSED);

        const int nbTris = tri->NbTriangles();
        for (int t = 1; t <= nbTris; ++t)
        {
            Poly_Triangle triIdx = tri->Triangle(t);

            int n1, n2, n3;
            triIdx.Get(n1, n2, n3);

            if (reversed) std::swap(n2, n3);

            gp_Pnt p1 = tri->Node(n1).Transformed(trsf);
            gp_Pnt p2 = tri->Node(n2).Transformed(trsf);
            gp_Pnt p3 = tri->Node(n3).Transformed(trsf);

            const unsigned base = verts->size();
            verts->push_back(osg::Vec3(p1.X(), p1.Y(), p1.Z()));
            verts->push_back(osg::Vec3(p2.X(), p2.Y(), p2.Z()));
            verts->push_back(osg::Vec3(p3.X(), p3.Y(), p3.Z()));

            indices->push_back(base + 0);
            indices->push_back(base + 1);
            indices->push_back(base + 2);

            if (norms)
            {
                osg::Vec3 a = (*verts)[base + 1] - (*verts)[base + 0];
                osg::Vec3 b = (*verts)[base + 2] - (*verts)[base + 0];
                osg::Vec3 n = a ^ b;
                n.normalize();
                norms->push_back(n);
                norms->push_back(n);
                norms->push_back(n);
            }
        }

        auto geom = makeGeom();
        geom->setVertexArray(verts);
        geom->addPrimitiveSet(indices);

        if (norms)
        {
            geom->setNormalArray(norms, osg::Array::BIND_PER_VERTEX);
        }

        auto geode = new osg::Geode();
        geode->addDrawable(geom);
        return geode;
    }
}