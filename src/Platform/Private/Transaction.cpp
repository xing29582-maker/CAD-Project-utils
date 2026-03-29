#include "Transaction.h"
#include "Document.h"
#include "IObject.h"

using namespace cadutils;

cadutils::Transaction::Transaction(std::weak_ptr<Document> doc)
	:m_doc(doc)
{
	Start();
	m_doc.lock()->SetCurrentTransaction(this);
}

cadutils::Transaction::~Transaction() noexcept
{
	m_doc.lock()->SetCurrentTransaction(nullptr);
}

void cadutils::Transaction::OnPropertyChanging(cadutils::ObjectId objId, cadutils::PropertyId propId, const cadutils::AnyValue& oldValue)
{
    ChangeKey key{ objId, propId };
    auto it = m_changes.find(key);
    if (it != m_changes.end()) 
    {
        return; // 同一事务内同一属性，old 只记第一次
    }
    ChangeRec rec;
    rec.oldValue = oldValue;
    m_changes[key] = rec;
}

void cadutils::Transaction::OnPropertyChanged(cadutils::ObjectId objId, cadutils::PropertyId propId, const cadutils::AnyValue& newValue)
{
    ChangeKey key{ objId, propId };
    auto it = m_changes.find(key);
    if (it == m_changes.end())
    {
        return;
    }
    it->second.newValue = newValue;
}

void cadutils::Transaction::Commit()
{
}

void cadutils::Transaction::RollBack()
{
    if (!m_doc.lock())
        return;
    // 回放期间禁止生成新历史（并且通常也不希望触发“再记录一次”）
    Document::ExecStateGuard g(*m_doc.lock().get(), Document::ExecState::Undo);

    for (const std::pair<ChangeKey, ChangeRec>& change : m_changes)
    {
        m_doc.lock()->ApplyPropertySilent(change.first.objId, change.first.propId, change.second.oldValue);
    }
}
