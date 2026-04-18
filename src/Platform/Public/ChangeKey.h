#pragma once

#include "NameDefine.h"

#include <memory>

namespace cadutils
{
    class IObject;

    struct ChangeKey
    {
        cadutils::ObjectId  objId{};
        cadutils::PropertyId propId{};

        bool operator==(const ChangeKey& o) const noexcept
        {
            return objId == o.objId && propId == o.propId;
        }
    };

    // hash
    struct ChangeKeyHash
    {
        std::size_t operator()(const ChangeKey& k) const noexcept
        {
            std::size_t h1 = std::hash<cadutils::ObjectId>{}(k.objId);
            std::size_t h2 = std::hash<cadutils::PropertyId>{}(k.propId);
            return h1 ^ (h2 + 0x9e3779b97f4a7c15ull + (h1 << 6) + (h1 >> 2));
        }
    };

    struct ChangeRec
    {
        cadutils::AnyValue oldValue;
        cadutils::AnyValue newValue;
    };

    // --- Object-level change support ---

    enum class ChangeType : uint8_t
    {
        PropertyChange,   // property modification
        ObjectAdd,        // object was added to document
        ObjectRemove      // object was removed from document
    };

    struct ChangeEntry
    {
        ChangeType type = ChangeType::PropertyChange;
        ObjectId objId{};

        // PropertyChange fields
        PropertyId propId{};
        AnyValue oldValue;
        AnyValue newValue;

        // ObjectAdd / ObjectRemove: holds the object alive via shared_ptr
        std::shared_ptr<IObject> object;
    };
}