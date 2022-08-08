#include "object.h"

#include "garbageCollector.h"

void interpreter::ObjectValue::SetProperty(std::string name, Value value)
{
	volatile GarbageCollector::GCInstructionsBatch batch;

	m_properties[name] = value;
	Value& tmp = m_properties[name];

	tmp.SetImplicitRef(this);
}

interpreter::Value interpreter::ObjectValue::GetProperty(std::string name) const
{
	std::map<std::string, Value>::const_iterator it = m_properties.find(name);

	if (it == m_properties.end()) {
		return Value();
	}

	return it->second;
}
