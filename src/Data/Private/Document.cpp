#include "Document.h"
#include "Object.h"
#include "TypeMeta.h"
#include "PropertyDescriptor.h"
#include "JsonSerializer.h"
#include <fstream>

using namespace cadutils;
using namespace std;

cadutils::Document::Document(const std::string& name)
	: m_name(name)
	, m_nextId(1)
	, m_selectedId(0)
{
}

const std::string& cadutils::Document::name() const
{
	return m_name;
}

void cadutils::Document::add(const std::shared_ptr<IObject>& obj)
{
	m_objects.emplace(m_nextId, obj);
	std::shared_ptr<Object> obj2 = std::dynamic_pointer_cast<Object>(obj);
	obj2->m_objId.set(m_nextId);
	++m_nextId;
	obj2->SetOwnerDoc(this);

	// Notify change sink (for transaction recording)
	if (!IsReplaying() && m_changeSink)
	{
		m_changeSink->OnObjectAdded(obj->GetObjectId(), obj);
	}
}

void cadutils::Document::addWithId(const std::shared_ptr<IObject>& obj, ObjectId id)
{
	m_objects.emplace(id, obj);
	std::shared_ptr<Object> obj2 = std::dynamic_pointer_cast<Object>(obj);
	if (obj2)
	{
		obj2->m_objId.SetValueSilent(id);
		obj2->SetOwnerDoc(this);
	}

	// Update next ID if necessary
	if (id >= m_nextId)
	{
		m_nextId = id + 1;
	}

	// Note: No change sink notification during deserialization
}

bool cadutils::Document::remove(ObjectId id)
{
	auto it = m_objects.find(id);
	if (it == m_objects.end())
		return false;

	std::shared_ptr<IObject> obj = it->second;

	// Notify change sink before removal
	if (!IsReplaying() && m_changeSink)
	{
		m_changeSink->OnObjectRemoved(id, obj);
	}

	m_objects.erase(it);

	// Clear selection if the removed object was selected
	if (m_selectedId == id)
		m_selectedId = 0;

	return true;
}

bool cadutils::Document::restore(const std::shared_ptr<IObject>& obj)
{
	ObjectId id = obj->GetObjectId();
	if (m_objects.find(id) != m_objects.end())
		return false; // already exists

	m_objects.emplace(id, obj);

	// Re-bind owner doc
	std::shared_ptr<Object> obj2 = std::dynamic_pointer_cast<Object>(obj);
	if (obj2)
		obj2->SetOwnerDoc(this);

	return true;
}

std::shared_ptr<IObject> cadutils::Document::GetobjectById(ObjectId id) const
{
	auto idIter = m_objects.find(id);
	if (idIter != m_objects.end())
	{
		return idIter->second;
	}
	return std::shared_ptr<IObject>();
}

std::vector<std::shared_ptr<IObject>> Document::GetObjects() const
{
	std::vector<std::shared_ptr<IObject>> resObj;
	for (const auto& obj : m_objects)
	{
		resObj.emplace_back(obj.second);
	}
	return resObj;
}

void cadutils::Document::SetSelected(ObjectId id)
{
	m_selectedId = id;
}

ObjectId cadutils::Document::GetSelected() const
{
	return m_selectedId;
}

std::vector<DirtyItem> cadutils::Document::ConsumeDirty()
{
	std::vector<DirtyItem> out;
	out.reserve(m_dirty.size());
	for (auto& [id, flags] : m_dirty) out.push_back({ id, flags });
	m_dirty.clear();
	return out;
}

void cadutils::Document::SetChangeSink(IPropertyChangeSink* sink)
{
	m_changeSink = sink;
}

bool cadutils::Document::ApplyPropertySilent(ObjectId objId, PropertyId propId, const AnyValue& v)
{
	shared_ptr<IObject> objI = GetobjectById(objId);
	if (!objI) return false;

	const TypeMeta& meta = objI->GetTypeMeta();

	const PropertyDescriptor* desc = meta.FindById(propId);
	if (!desc)
		return false;

	if (!desc->applyAny)
		return false;

	return desc->applyAny(*objI, v);
}

void cadutils::Document::OnObjectDirty(ObjectId id, DirtyFlags flags)
{
	m_dirty[id] = flags;
}

void cadutils::Document::OnPropertyChanging(IObject& obj, PropertyBase& prop)
{
	if (IsReplaying())
		return;

	if (m_changeSink)
	{
		m_changeSink->OnPropertyChanging(
			obj.GetObjectId(),
			prop.id(),
			prop.Value()
		);
	}
}

bool cadutils::Document::SaveToFile(const std::string& path) const
{
	try
	{
		nlohmann::json j = JsonSerializer::SerializeDocument(*this);
		std::ofstream file(path);
		if (!file.is_open())
			return false;

		file << j.dump(2);  // Pretty print with 2-space indent
		file.close();
		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool cadutils::Document::LoadFromFile(const std::string& path)
{
	try
	{
		std::ifstream file(path);
		if (!file.is_open())
			return false;

		nlohmann::json j;
		file >> j;
		file.close();

		// Clear current document
		m_objects.clear();
		m_dirty.clear();
		m_selectedId = 0;

		// Deserialize
		return JsonSerializer::DeserializeDocument(*this, j);
	}
	catch (...)
	{
		return false;
	}
}

void cadutils::Document::OnPropertyChanged(IObject& obj, PropertyBase& prop)
{
	if (IsReplaying())
		return;

	OnObjectDirty(obj.GetObjectId(), prop.flags());

	if (m_changeSink)
	{
		m_changeSink->OnPropertyChanged(
			obj.GetObjectId(),
			prop.id(),
			prop.Value()
		);
	}
}
