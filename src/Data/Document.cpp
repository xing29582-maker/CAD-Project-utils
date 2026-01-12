#include"Document.h"

using namespace cadutils;

cadutils::Document::Document(const std::string& name)
	: m_name(name) 
	, m_nextId(1)
{
}

const std::string& cadutils::Document::name() const
{
	return m_name;
}

void cadutils::Document::add(std::shared_ptr<Object> obj)
{
	m_objects.emplace(m_nextId, obj);
	obj->m_ObjId = m_nextId;
	++m_nextId;
}

const std::weak_ptr<Object> cadutils::Document::GetobjectById(int64_t id) const
{
	auto idIter = m_objects.find(id);
	if (idIter != m_objects.end())
	{
		return idIter->second;
	}
	return std::weak_ptr<Object>();
}

const std::unordered_map<int64_t, std::shared_ptr<Object>>& Document::GetObjects() const
{
	return m_objects;
}
