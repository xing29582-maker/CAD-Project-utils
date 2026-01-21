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
        const std::weak_ptr<Object> GetobjectById(ObjectId id) const;
        const std::vector<std::shared_ptr<Object>> GetObjects() const;

    private:
        ObjectId m_nextId;
        std::string m_name;
        std::unordered_map<ObjectId, std::shared_ptr<Object>> m_objects;
    };
}