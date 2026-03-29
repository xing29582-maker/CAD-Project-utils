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
    class Transaction;

    class CADUTILS_DATA_API Document : public IDirtySink
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
        void SetCurrentTransaction(IPropertyChangeSink * transaction);
        IPropertyChangeSink* GetCurrentTransaction() const { return m_transaction; }
        bool ApplyPropertySilent(ObjectId objId, PropertyId propId, const AnyValue& v);
        bool IsUndoing() const noexcept { return m_execState == ExecState::Undo; }
        bool IsRedoing() const noexcept { return m_execState == ExecState::Redo; }
        bool IsReplaying() const noexcept { return m_execState != ExecState::Normal; }
        void Undo();
        void Redo();
        void OnPropertyChanging(IObject& obj, PropertyBase& prop);
        void OnPropertyChanged(IObject& obj, PropertyBase& prop);
    public:
        void OnObjectDirty(ObjectId id,  DirtyFlags flags) override;
    private:
        enum class ExecState 
        {
            Normal,   // ”√ªß±‡º≠
            Undo,
            Redo
        };

        class ExecStateGuard
        {
        public:
            ExecStateGuard(Document& doc, Document::ExecState state)
                : m_doc(doc), m_prev(doc.m_execState)
            {
                m_doc.m_execState = state;
            }

            ~ExecStateGuard() {
                m_doc.m_execState = m_prev;
            }

        private:
            Document& m_doc;
            Document::ExecState m_prev;
        };

        friend class Transaction;
    private:
        IPropertyChangeSink* m_transaction;
        std::unordered_map<ObjectId, DirtyFlags> m_dirty;
        ObjectId m_selectedId;
        ObjectId m_nextId;
        std::string m_name;
        std::unordered_map<ObjectId, std::shared_ptr<IObject>> m_objects;
        ExecState m_execState = ExecState::Normal;
    };
}