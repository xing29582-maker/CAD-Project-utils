#include "Transaction.h"

using namespace cadutils;

void Transaction::OnPropertyChanging(ObjectId objId, PropertyId propId, const AnyValue& oldValue)
{
    ChangeKey key{ objId, propId };
    auto it = m_propIndex.find(key);
    if (it != m_propIndex.end())
    {
        return; // same property in same transaction: keep first old value
    }

    ChangeEntry entry;
    entry.type = ChangeType::PropertyChange;
    entry.objId = objId;
    entry.propId = propId;
    entry.oldValue = oldValue;

    m_propIndex[key] = m_entries.size();
    m_entries.push_back(std::move(entry));
}

void Transaction::OnPropertyChanged(ObjectId objId, PropertyId propId, const AnyValue& newValue)
{
    ChangeKey key{ objId, propId };
    auto it = m_propIndex.find(key);
    if (it == m_propIndex.end())
    {
        return;
    }
    m_entries[it->second].newValue = newValue;
}

void Transaction::OnObjectAdded(ObjectId objId, const std::shared_ptr<IObject>& obj)
{
    ChangeEntry entry;
    entry.type = ChangeType::ObjectAdd;
    entry.objId = objId;
    entry.object = obj;
    m_entries.push_back(std::move(entry));
}

void Transaction::OnObjectRemoved(ObjectId objId, const std::shared_ptr<IObject>& obj)
{
    ChangeEntry entry;
    entry.type = ChangeType::ObjectRemove;
    entry.objId = objId;
    entry.object = obj;
    m_entries.push_back(std::move(entry));
}
