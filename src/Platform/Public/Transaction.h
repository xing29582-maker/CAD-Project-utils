#pragma once

#include "PlatformExport.h"
#include "IPropertyChangeSink.h"
#include "ChangeKey.h"

#include <memory>
#include <unordered_map>

namespace cadutils
{
    class Document;

    class CADUTILS_PLATFORM_API Transaction : public IPropertyChangeSink
    {
    public:
        Transaction(std::weak_ptr<Document> doc);
        virtual ~Transaction() noexcept;
        virtual void OnPropertyChanging(cadutils::ObjectId objId,
            cadutils::PropertyId propId,
            const cadutils::AnyValue& oldValue) override;

        virtual void OnPropertyChanged(cadutils::ObjectId objId,
            cadutils::PropertyId propId,
            const cadutils::AnyValue& newValue) override;
        // 事务开始时的状态
        void Start()
        {
            // 清空操作历史
            m_changes.clear();
        }

        // 提交事务（执行所有操作）
        void Commit();

        // 撤销事务（撤销所有操作）
        void RollBack();

    private:
        std::weak_ptr<Document> m_doc;
        std::unordered_map<ChangeKey, ChangeRec, ChangeKeyHash> m_changes;
    };
}