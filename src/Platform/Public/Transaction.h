#pragma once

#include "PlatformExport.h"
#include "IPropertyChangeSink.h"
#include "ChangeKey.h"

#include <memory>
#include <vector>
#include <unordered_map>

namespace cadutils
{
    class CADUTILS_PLATFORM_API Transaction : public IPropertyChangeSink
    {
    public:
        Transaction() = default;
        virtual ~Transaction() noexcept = default;

        // Property-level
        virtual void OnPropertyChanging(cadutils::ObjectId objId,
            cadutils::PropertyId propId,
            const cadutils::AnyValue& oldValue) override;

        virtual void OnPropertyChanged(cadutils::ObjectId objId,
            cadutils::PropertyId propId,
            const cadutils::AnyValue& newValue) override;

        // Object-level
        virtual void OnObjectAdded(cadutils::ObjectId objId,
            const std::shared_ptr<IObject>& obj) override;

        virtual void OnObjectRemoved(cadutils::ObjectId objId,
            const std::shared_ptr<IObject>& obj) override;

        const std::vector<ChangeEntry>& GetEntries() const noexcept
        {
            return m_entries;
        }

        bool IsEmpty() const noexcept { return m_entries.empty(); }

        void Clear() { m_entries.clear(); m_propIndex.clear(); }

    private:
        std::vector<ChangeEntry> m_entries;
        // Index for property changes: ChangeKey → index in m_entries
        // Used to find existing entry for same property (only record first oldValue)
        std::unordered_map<ChangeKey, size_t, ChangeKeyHash> m_propIndex;
    };
}