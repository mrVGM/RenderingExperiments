#include "renderObject.h"
#include "garbageCollector.h"

void rendering::RenderObject::RegisterProperty(std::string name, interpreter::Value* value)
{
	m_props[name] = value;
}

void rendering::RenderObject::SetProperty(std::string name, interpreter::Value value)
{
	std::map<std::string, interpreter::Value*>::iterator it = m_props.find(name);
	if (it == m_props.end()) {
		return;
	}

	volatile interpreter::GarbageCollector::GCInstructionsBatch batch;

	interpreter::Value& val = *(it->second);
	val = value;
	val.SetImplicitRef(this);
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

