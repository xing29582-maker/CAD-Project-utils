#include"Document.h"
#include "Object.h"

using namespace cadutils;

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

void cadutils::Document::add(const std::shared_ptr<IObject> &obj)
{
	m_objects.emplace(m_nextId, obj);
	std::dynamic_pointer_cast<Object>(obj)->m_objId.set(m_nextId);
	++m_nextId;
	obj->OnAddedToDocument(*this);
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

void cadutils::Document::OnPropertyChanged(ObjectId id,  DirtyFlags flags)
{
	m_dirty[id] = flags;
}

