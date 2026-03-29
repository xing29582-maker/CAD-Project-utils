#pragma once

#include "NameDefine.h"

namespace cadutils
{
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

}