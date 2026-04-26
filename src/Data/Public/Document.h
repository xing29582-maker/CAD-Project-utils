#pragma once

#include "DataExport.h"
#include "IDirtySink.h"
#include "DirtyFlags.h"
#include "PropertyRegistry.h"

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace cadutils
{
    struct DirtyItem { ObjectId id; DirtyFlags flags; };

    class IObject;
    class IPropertyChangeSink;

    class CADUTILS_DATA_API Document : public IDirtySink
    {
    public:
        explicit Document(const std::string& name);
        virtual ~Document() = default;
        const std::string& name() const;

        // Object management
        void add(const std::shared_ptr<IObject>& obj);
        void addWithId(const std::shared_ptr<IObject>& obj, ObjectId id);  // 新增：用于加载时恢复对象 ID
        bool remove(ObjectId id);
        bool restore(const std::shared_ptr<IObject>& obj);
        std::shared_ptr<IObject> GetobjectById(ObjectId id) const;
        std::vector<std::shared_ptr<IObject>> GetObjects() const;

        // Persistence
        bool SaveToFile(const std::string& path) const;
        bool LoadFromFile(const std::string& path);

        void SetSelected(ObjectId id);
        ObjectId GetSelected() const;
        std::vector<DirtyItem> ConsumeDirty();

        // Change sink (used by TransactionManager to wire up a Transaction)
        void SetChangeSink(IPropertyChangeSink* sink);
        IPropertyChangeSink* GetChangeSink() const { return m_changeSink; }

        bool ApplyPropertySilent(ObjectId objId, PropertyId propId, const AnyValue& v);

        // Exec state queries
        bool IsUndoing() const noexcept { return m_execState == ExecState::Undo; }
        bool IsRedoing() const noexcept { return m_execState == ExecState::Redo; }
        bool IsReplaying() const noexcept { return m_execState != ExecState::Normal; }

        void OnPropertyChanging(IObject& obj, PropertyBase& prop);
        void OnPropertyChanged(IObject& obj, PropertyBase& prop);

        // ID management (for deserialization)
        void setNextId(ObjectId id) { m_nextId = id; }
        ObjectId getNextId() const { return m_nextId; }

    public:
        void OnObjectDirty(ObjectId id, DirtyFlags flags) override;

    public:
        // ExecState management — public so TransactionManager can use it
        enum class ExecState
        {
            Normal,
            Undo,
            Redo
        };

        class ExecStateGuard
        {
        public:
            ExecStateGuard(Document& doc, ExecState state)
                : m_doc(doc), m_prev(doc.m_execState)
            {
                m_doc.m_execState = state;
            }

            ~ExecStateGuard() {
                m_doc.m_execState = m_prev;
            }

        private:
            Document& m_doc;
            ExecState m_prev;
        };

    private:
        IPropertyChangeSink* m_changeSink = nullptr;
        std::unordered_map<ObjectId, DirtyFlags> m_dirty;
        ObjectId m_selectedId;
        ObjectId m_nextId;
        std::string m_name;
        std::unordered_map<ObjectId, std::shared_ptr<IObject>> m_objects;
        ExecState m_execState = ExecState::Normal;
    };
}