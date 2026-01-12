#pragma once
#include <osg/Node>
#include <TopoDS_Shape.hxx>

struct TessellationOptions
{
    double deflection = 0.5; // ÏÈ±ðÌ«Ð¡
    bool computeNormals = true;
};

osg::ref_ptr<osg::Node> BuildOsgNodeFromShape(const TopoDS_Shape& shape,
    const TessellationOptions& opt);
