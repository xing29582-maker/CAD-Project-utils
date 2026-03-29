#pragma once


#include "TypeMeta.h"

namespace cadutils
{
    class MetaRegistry
    {
    public:
        static MetaRegistry& Instance()
        {
            static MetaRegistry g;
            return g;
        }

        void Register(const TypeMeta* meta)
        {
            m_types[meta->typeId] = meta;
        }

        const TypeMeta* Find(cadutils::PropertyId typeId) const
        {
            auto it = m_types.find(typeId);
            return it == m_types.end() ? nullptr : it->second;
        }

    private:
        std::unordered_map<cadutils::PropertyId, const TypeMeta*> m_types;
    };
}
