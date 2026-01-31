#pragma once

#include"DataExport.h"
#include"IObject.h"

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace cadutils
{

    class CADUTILS_DATA_API Document
    {
    public:
        explicit Document(const std::string& name);

        const std::string& name() const;
        void add(std::shared_ptr<IObject> obj);
        const std::shared_ptr<IObject> GetobjectById(ObjectId id) const;
        const std::vector<std::shared_ptr<IObject>> GetObjects() const;
        void SetSelected(ObjectId id);
        ObjectId GetSelected() const;
        void MarkDirty(ObjectId id);
        std::vector<ObjectId> ConsumeDirtyIds();
    private:
        std::vector<ObjectId> m_dirtyIds;
        ObjectId m_selectedId;
        ObjectId m_nextId;
        std::string m_name;
        std::unordered_map<ObjectId, std::shared_ptr<IObject>> m_objects;
    };
}