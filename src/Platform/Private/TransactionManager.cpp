#include "TransactionManager.h"
#include "Transaction.h"
#include "Document.h"
#include "IObject.h"
#include "TypeMeta.h"
#include "PropertyDescriptor.h"
#include "ChangeKey.h"

using namespace cadutils;

TransactionManager::TransactionManager(std::weak_ptr<Document> doc)
    : m_doc(doc)
{
}

bool TransactionManager::BeginTransaction()
{
    if (m_active)
        return false;

    auto doc = m_doc.lock();
    if (!doc)
        return false;

    m_active = std::make_shared<Transaction>();
    doc->SetChangeSink(m_active.get());
    return true;
}

bool TransactionManager::Commit()
{
    if (!m_active)
        return false;

    auto doc = m_doc.lock();
    if (doc)
        doc->SetChangeSink(nullptr);

    if (!m_active->IsEmpty())
    {
        m_undoStack.push_back(std::move(m_active));
        m_redoStack.clear();
    }

    m_active.reset();
    return true;
}

bool TransactionManager::RollBack()
{
    if (!m_active)
        return false;

    auto doc = m_doc.lock();
    if (!doc)
        return false;

    doc->SetChangeSink(nullptr);

    // Restore: apply in reverse order
    const auto& entries = m_active->GetEntries();
    Document::ExecStateGuard guard(*doc, Document::ExecState::Undo);

    for (auto it = entries.rbegin(); it != entries.rend(); ++it)
    {
        const ChangeEntry& e = *it;
        switch (e.type)
        {
        case ChangeType::PropertyChange:
            doc->ApplyPropertySilent(e.objId, e.propId, e.oldValue);
            break;
        case ChangeType::ObjectAdd:
            // Undo add = remove
            doc->remove(e.objId);
            break;
        case ChangeType::ObjectRemove:
            // Undo remove = restore
            doc->restore(e.object);
            break;
        }
    }

    m_active.reset();
    return true;
}

bool TransactionManager::Undo()
{
    if (m_undoStack.empty())
        return false;

    auto doc = m_doc.lock();
    if (!doc)
        return false;

    auto tx = std::move(m_undoStack.back());
    m_undoStack.pop_back();

    ApplyTransaction(*tx, true);

    m_redoStack.push_back(std::move(tx));
    return true;
}

bool TransactionManager::Redo()
{
    if (m_redoStack.empty())
        return false;

    auto doc = m_doc.lock();
    if (!doc)
        return false;

    auto tx = std::move(m_redoStack.back());
    m_redoStack.pop_back();

    ApplyTransaction(*tx, false);

    m_undoStack.push_back(std::move(tx));
    return true;
}

void TransactionManager::ApplyTransaction(const Transaction& tx, bool useOldValues)
{
    auto doc = m_doc.lock();
    if (!doc)
        return;

    Document::ExecStateGuard guard(*doc,
        useOldValues ? Document::ExecState::Undo : Document::ExecState::Redo);

    const auto& entries = tx.GetEntries();

    if (useOldValues)
    {
        // Undo: apply in reverse order
        for (auto it = entries.rbegin(); it != entries.rend(); ++it)
        {
            const ChangeEntry& e = *it;
            switch (e.type)
            {
            case ChangeType::PropertyChange:
                doc->ApplyPropertySilent(e.objId, e.propId, e.oldValue);
                // Write back dirty flags
                {
                    auto obj = doc->GetobjectById(e.objId);
                    if (obj)
                    {
                        const TypeMeta& meta = obj->GetTypeMeta();
                        const PropertyDescriptor* desc = meta.FindById(e.propId);
                        if (desc)
                            doc->OnObjectDirty(e.objId, static_cast<DirtyFlags>(desc->flags));
                    }
                }
                break;
            case ChangeType::ObjectAdd:
                // Undo add = remove the object (shared_ptr kept alive in ChangeEntry)
                doc->remove(e.objId);
                break;
            case ChangeType::ObjectRemove:
                // Undo remove = restore the object from ChangeEntry
                doc->restore(e.object);
                doc->OnObjectDirty(e.objId, DirtyFlags::Geometry);
                break;
            }
        }
    }
    else
    {
        // Redo: apply in forward order
        for (const auto& e : entries)
        {
            switch (e.type)
            {
            case ChangeType::PropertyChange:
                doc->ApplyPropertySilent(e.objId, e.propId, e.newValue);
                {
                    auto obj = doc->GetobjectById(e.objId);
                    if (obj)
                    {
                        const TypeMeta& meta = obj->GetTypeMeta();
                        const PropertyDescriptor* desc = meta.FindById(e.propId);
                        if (desc)
                            doc->OnObjectDirty(e.objId, static_cast<DirtyFlags>(desc->flags));
                    }
                }
                break;
            case ChangeType::ObjectAdd:
                // Redo add = restore the object
                doc->restore(e.object);
                doc->OnObjectDirty(e.objId, DirtyFlags::Geometry);
                break;
            case ChangeType::ObjectRemove:
                // Redo remove = remove again
                doc->remove(e.objId);
                break;
            }
        }
    }
}