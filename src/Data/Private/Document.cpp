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

void cadutils::Document::add(std::shared_ptr<IObject> obj)
{
	m_objects.emplace(m_nextId, obj);
	std::dynamic_pointer_cast<Object>(obj)->m_ObjId = m_nextId;
	++m_nextId;
}

const std::shared_ptr<IObject> cadutils::Document::GetobjectById(ObjectId id) const
{
	auto idIter = m_objects.find(id);
	if (idIter != m_objects.end())
	{
		return idIter->second;
	}
	return std::shared_ptr<IObject>();
}

const std::vector<std::shared_ptr<IObject>> Document::GetObjects() const
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

void cadutils::Document::MarkDirty(ObjectId id)
{
	m_dirtyIds.emplace_back(id);
}

std::vector<ObjectId> cadutils::Document::ConsumeDirtyIds()
{
	std::vector<ObjectId> dirty = m_dirtyIds;
	m_dirtyIds.clear();
	return dirty;
}

