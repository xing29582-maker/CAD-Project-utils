#include"Document.h"

using namespace cadutils;

cadutils::Document::Document(const std::string& name)
	: m_name(name) 
{
}

const std::string& cadutils::Document::name() const
{
	return m_name;
}

void cadutils::Document::add(std::shared_ptr<Object> obj)
{
	m_objects.emplace_back(obj);
}

const std::vector<std::shared_ptr<Object>>& cadutils::Document::objects() const
{
	return m_objects;
}
