#pragma once

#include "DataExport.h"
#include "IChangeSink.h"
#include "DirtyFlags.h"

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace cadutils
{
    struct DirtyItem { ObjectId id; DirtyFlags flags; };

    class IObject;

    class CADUTILS_DATA_API Document : public IChangeSink
    {
    public:
        explicit Document(const std::string& name);
        virtual ~Document() = default;
        const std::string& name() const;
        void add(const std::shared_ptr<IObject> &obj);
        std::shared_ptr<IObject> GetobjectById(ObjectId id) const;
        std::vector<std::shared_ptr<IObject>> GetObjects() const;
        void SetSelected(ObjectId id);
        ObjectId GetSelected() const;
        std::vector<DirtyItem>  ConsumeDirty();
    public:
        void OnPropertyChanged(ObjectId id,  DirtyFlags flags) override;
    private:
        std::unordered_map<ObjectId, DirtyFlags> m_dirty;
        ObjectId m_selectedId;
        ObjectId m_nextId;
        std::string m_name;
        std::unordered_map<ObjectId, std::shared_ptr<IObject>> m_objects;
    };
}