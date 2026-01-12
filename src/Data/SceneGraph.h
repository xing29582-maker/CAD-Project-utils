#pragma once

#include "DataExport.h"

#include <unordered_map>
#include <memory>
#include <osg/Group>
#include "RenderItem.h"
#include <memory>


namespace cadutils
{
    class Document;
    class Object;
    struct IGeometrySource;
}

class CADUTILS_DATA_API SceneGraph
{
public:
    SceneGraph();

    osg::Group* root() const { return _root.get(); }
    void syncFromDocument(const std::weak_ptr<cadutils::Document> doc);

private:
    osg::ref_ptr<osg::Group> _root;
    std::unordered_map<uint64_t, RenderItem> _items; // objectId -> render item
};
