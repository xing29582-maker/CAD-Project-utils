#pragma once

#include "TypeMeta.h"
#include <string>
#include <memory>

namespace cadutils
{
    class IObject;

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
            // 新增：同时按类型名注册
            if (meta->typeName) {
                m_typesByName[meta->typeName] = meta;
            }
        }

        const TypeMeta* Find(cadutils::PropertyId typeId) const
        {
            auto it = m_types.find(typeId);
            return it == m_types.end() ? nullptr : it->second;
        }

        // 新增：通过类型名查找
        const TypeMeta* FindByName(const char* typeName) const
        {
            if (!typeName) return nullptr;
            auto it = m_typesByName.find(typeName);
            return it == m_typesByName.end() ? nullptr : it->second;
        }

        // 新增：通过类型名创建对象
        std::shared_ptr<IObject> CreateByTypeName(const char* typeName) const
        {
            const TypeMeta* meta = FindByName(typeName);
            if (!meta || !meta->creator) {
                return nullptr;
            }
            return meta->creator();
        }

    private:
        std::unordered_map<cadutils::PropertyId, const TypeMeta*> m_types;
        std::unordered_map<std::string, const TypeMeta*> m_typesByName;  // 新增
    };
}
