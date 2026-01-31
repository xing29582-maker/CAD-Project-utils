#pragma once

#include <unordered_map>
#include <memory>

#include "IObject.h"


namespace cadutils
{
    class IGraphicsNode;

    class GraphicsScene 
    {
    public:
        std::shared_ptr<IGraphicsNode> GetOrCreate(ObjectId id);
        std::shared_ptr<IGraphicsNode> Find(ObjectId id) const;

        // 用于遍历给 view_osg 创建/更新节点
        std::vector<std::shared_ptr<IGraphicsNode>> GetItems() const;
        const std::unordered_map<ObjectId, std::shared_ptr<IGraphicsNode>>& GetAllNodesWithId() const;
        void Remove(ObjectId id);
        void MarkDirty(ObjectId id);
        std::unordered_map<ObjectId, std::shared_ptr<IGraphicsNode>> ConsumeDirtyIds();
    private:
        std::unordered_map<ObjectId, std::shared_ptr<IGraphicsNode>> m_dirtyItems;
        std::unordered_map<ObjectId, std::shared_ptr<IGraphicsNode>> m_items;
    };
}