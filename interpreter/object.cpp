#include "object.h"

#include "garbageCollector.h"

void interpreter::ObjectValue::SetProperty(std::string name, Value value)
{
	volatile GarbageCollector::GCInstructionsBatch batch;

	std::map<std::string, interpreter::Value>::iterator it = m_properties.find(name);
	if (it != m_properties.end()) {
		m_properties[name] = value;
		return;
	}

	m_properties[name] = Value();
	m_properties[name].SetImplicitRef(*this);
	m_properties[name] = value;
}

interpreter::Value interpreter::ObjectValue::GetProperty(std::string name) const
{
	std::map<std::string, Value>::const_iterator it = m_properties.find(name);

	if (it == m_properties.end()) {
		return Value();
	}

	return it->second;
}

interpreter::Value interpreter::ObjectValue::Create()
{
	ObjectValue* objectValue = new ObjectValue();
	return Value(*objectValue);
}

interpreter::ObjectValue::ObjectValue()
{
}
