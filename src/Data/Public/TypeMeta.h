#pragma once

#include"NameDefine.h"
#include"PropertyRegistry.h"
#include"PropertyDescriptor.h"

#include<vector>
#include<unordered_map>
namespace cadutils
{
    struct TypeMeta
    {
        ObjTypeId typeId = 0;
        const char* typeName = nullptr;

        std::vector<PropertyDescriptor> properties;

        std::unordered_map<PropertyId, const PropertyDescriptor*> idMap;
        std::unordered_map<std::size_t, const PropertyDescriptor*> offsetMap;
        std::unordered_map<std::string_view, const PropertyDescriptor*> nameMap;

        void BuildIndices()
        {
            idMap.clear();
            offsetMap.clear();
            nameMap.clear();

            for (const auto& p : properties)
            {
                idMap.emplace(p.id, &p);
                offsetMap.emplace(p.offset, &p);
                nameMap.emplace(p.name, &p);
            }
        }

        const PropertyDescriptor* FindById(PropertyId id) const
        {
            auto it = idMap.find(id);
            return it == idMap.end() ? nullptr : it->second;
        }

        const PropertyDescriptor* FindByOffset(std::size_t offset) const
        {
            auto it = offsetMap.find(offset);
            return it == offsetMap.end() ? nullptr : it->second;
        }

        const PropertyDescriptor* FindByName(std::string_view name) const
        {
            auto it = nameMap.find(name);
            return it == nameMap.end() ? nullptr : it->second;
        }
    };
}