#include "JsonSerializer.h"
#include "Document.h"
#include "Object.h"
#include "IObject.h"
#include "TypeMeta.h"
#include "PropertyDescriptor.h"
#include "MetaRegistry.h"

using namespace cadutils;
using json = nlohmann::json;

json JsonSerializer::SerializeObject(const std::shared_ptr<IObject>& obj)
{
    json jObj;

    const TypeMeta& meta = obj->GetTypeMeta();
    jObj["type"] = meta.typeName;
    jObj["id"] = obj->GetObjectId();

    json jProps;
    for (const auto& desc : meta.properties)
    {
        // Skip non-serializable properties
        if (!desc.serializable)
            continue;

        // Get property pointer via offset
        auto* propBase = reinterpret_cast<const PropertyBase*>(
            reinterpret_cast<const char*>(obj.get()) + desc.offset);

        AnyValue val = propBase->Value();
        if (!val.text.empty())
        {
            jProps[desc.name] = val.text;
        }
    }

    jObj["properties"] = jProps;
    return jObj;
}

std::shared_ptr<IObject> JsonSerializer::DeserializeObject(const json& j)
{
    if (!j.contains("type") || !j.contains("id"))
        return nullptr;

    std::string typeName = j["type"].get<std::string>();
    ObjectId id = j["id"].get<ObjectId>();

    // Create object via MetaRegistry
    auto obj = MetaRegistry::Instance().CreateByTypeName(typeName.c_str());
    if (!obj)
        return nullptr;

    // Restore properties
    if (j.contains("properties"))
    {
        const TypeMeta& meta = obj->GetTypeMeta();
        const json& jProps = j["properties"];

        for (auto it = jProps.begin(); it != jProps.end(); ++it)
        {
            const std::string& propName = it.key();
            const std::string& propValue = it.value().get<std::string>();

            const PropertyDescriptor* desc = meta.FindByName(propName);
            if (desc && desc->applyAny && desc->serializable)
            {
                desc->applyAny(*obj, AnyValue(propValue));
            }
        }
    }

    return obj;
}

json JsonSerializer::SerializeDocument(const Document& doc)
{
    json j;
    j["version"] = 1;
    j["documentName"] = doc.name();

    json jObjects = json::array();
    auto objects = doc.GetObjects();
    
    // Find max id for nextObjectId
    ObjectId maxId = 0;
    for (const auto& obj : objects)
    {
        jObjects.push_back(SerializeObject(obj));
        if (obj->GetObjectId() > maxId)
            maxId = obj->GetObjectId();
    }

    j["nextObjectId"] = maxId + 1;
    j["objects"] = jObjects;
    return j;
}

bool JsonSerializer::DeserializeDocument(Document& doc, const json& j)
{
    if (!j.contains("version") || !j.contains("objects"))
        return false;

    int version = j["version"].get<int>();
    if (version != 1)
        return false;

    // Deserialize objects
    const json& jObjects = j["objects"];
    ObjectId maxId = 0;

    for (const auto& jObj : jObjects)
    {
        auto obj = DeserializeObject(jObj);
        if (!obj)
            continue;

        ObjectId id = jObj["id"].get<ObjectId>();
        if (id > maxId)
            maxId = id;

        // Use addWithId to restore the original object ID
        doc.addWithId(obj, id);
    }

    // Restore next ID counter
    if (j.contains("nextObjectId"))
    {
        ObjectId nextId = j["nextObjectId"].get<ObjectId>();
        if (nextId > maxId)
            doc.setNextId(nextId);
        else
            doc.setNextId(maxId + 1);
    }
    else
    {
        doc.setNextId(maxId + 1);
    }

    return true;
}