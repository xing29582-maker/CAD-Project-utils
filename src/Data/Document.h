#pragma once

#include"DataExport.h"
#include"Object.h"

#include <string>
#include <vector>
#include <memory>

namespace cadutils
{
    class CADUTILS_DATA_API Document
    {
    public:
        explicit Document(const std::string& name);

        const std::string& name() const;

        void add(std::shared_ptr<Object> obj);
        const std::vector<std::shared_ptr<Object>>& objects() const;

    private:
        std::string m_name;
        std::vector<std::shared_ptr<Object>> m_objects;
    };
}