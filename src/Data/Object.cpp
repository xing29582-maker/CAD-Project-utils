#include "Object.h"

using namespace cadutils;

cadutils::Object::Object(std::string name)
	: m_name(std::move(name)) 
{

}

const std::string& cadutils::Object::name() const
{
	return m_name;
}

const std::vector<Property>& cadutils::Object::properties() const
{
	return m_props;
}

void cadutils::Object::addProperty(std::string k, std::string v)
{
	m_props.emplace_back(std::move(k), std::move(v));
}

cadutils::Box::Box()
    : Object("Box") {
    addProperty("Type", "Box");
    addProperty("Length", "10");
    addProperty("Width", "20");
    addProperty("Height", "30");
}

cadutils::Sphere::Sphere()
    : Object("Sphere") {
    addProperty("Type", "Sphere");
    addProperty("Radius", "15");
}
