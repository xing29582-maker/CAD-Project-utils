#include "OsgBackend.h"
#include "IGraphicsNode.h"

#include <osg/Geometry>
#include <osg/Geode>

using namespace cadutils;

osg::ref_ptr<osg::Node> cadutils::OsgBackend::BuildNode(const std::shared_ptr<IGraphicsNode> node)
{
    const GeometryData& geoData = node->GetGeometryData();
    osg::Vec3Array *verts = new osg::Vec3Array();
    verts->reserve(geoData.positions.size());
    for (auto& p : geoData.positions)
        verts->push_back(osg::Vec3(p.GetX(), p.GetY(), p.GetZ()));

    osg::ref_ptr<osg::DrawElementsUInt> indices =
        new osg::DrawElementsUInt(GL_TRIANGLES);
    indices->reserve(geoData.indices.size());
    for (auto i : geoData.indices) 
        indices->push_back(i);

    auto osgGeom = new osg::Geometry();
    osgGeom->setVertexArray(verts);
    osgGeom->addPrimitiveSet(indices.get());

    if (!geoData.normals.empty())
    {
        auto norms = new osg::Vec3Array();
        norms->reserve(geoData.normals.size());
        for (auto& n : geoData.normals)
            norms->push_back(osg::Vec3(n.GetX(), n.GetY(), n.GetZ()));
        osgGeom->setNormalArray(norms, osg::Array::BIND_PER_VERTEX);
    }

    auto geode = new osg::Geode();
    geode->addDrawable(osgGeom);

    // 这里再根据 node 的 material/visibility 等设置 StateSet（略）
    return geode;
}