#pragma once

#include"DataExport.h"
#include"Object.h"

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace cadutils
{
    class CADUTILS_DATA_API Document
    {
    public:
        explicit Document(const std::string& name);

        const std::string& name() const;

        void add(std::shared_ptr<Object> obj);
        const std::weak_ptr<Object> GetobjectById(int64_t id) const;
        const std::unordered_map<int64_t, std::shared_ptr<Object>> &GetObjects() const;

    private:
        int64_t m_nextId;
        std::string m_name;
        std::unordered_map<int64_t, std::shared_ptr<Object>> m_objects;
    };
}