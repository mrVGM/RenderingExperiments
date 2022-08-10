#include "renderObject.h"

void rendering::RenderObject::RegisterProperty(std::string name, interpreter::Value* value)
{
	m_props[name] = value;
	value->SetImplicitRef(*this);
}

void rendering::RenderObject::SetProperty(std::string name, interpreter::Value value)
{
	std::map<std::string, interpreter::Value*>::iterator it = m_props.find(name);
	if (it == m_props.end()) {
		return;
	}

	interpreter::Value& val = *(it->second);
	val = value;
}


interpreter::Value rendering::RenderObject::GetProperty(std::string name) const
{
	std::map<std::string, interpreter::Value*>::const_iterator it = m_props.find(name);
	if (it == m_props.end()) {
		return interpreter::Value();
	}

	return *(it->second);
}

rendering::RenderObject::~RenderObject()
{
}

interpreter::Value rendering::RenderObject::Create()
{
	RenderObject* ro = new RenderObject();
	interpreter::Value res(*ro);

	return res;
}

rendering::RenderObject::RenderObject()
{
}

