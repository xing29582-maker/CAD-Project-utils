#pragma once

#include "DataExport.h"
#include <nlohmann/json.hpp>
#include <memory>
#include <string>

namespace cadutils
{
    class Document;
    class IObject;

    class JsonSerializer
    {
    public:
        // 序列化文档到 JSON
        static nlohmann::json SerializeDocument(const Document& doc);
        
        // 从 JSON 反序列化文档
        static bool DeserializeDocument(Document& doc, const nlohmann::json& j);
        
        // 序列化单个对象到 JSON
        static nlohmann::json SerializeObject(const std::shared_ptr<IObject>& obj);
        
        // 从 JSON 反序列化单个对象
        static std::shared_ptr<IObject> DeserializeObject(const nlohmann::json& j);
    };
}