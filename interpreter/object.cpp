#include "object.h"

#include "garbageCollector.h"

void interpreter::ObjectValue::SetProperty(std::string name, ValueWrapper value)
{
	volatile GarbageCollector::GCInstructionsBatch batch;

	m_properties[name] = value;
	ValueWrapper& tmp = m_properties[name];

	tmp.SetImplicitRef(this);
}

interpreter::ValueWrapper interpreter::ObjectValue::GetProperty(std::string name) const
{
	std::map<std::string, ValueWrapper>::const_iterator it = m_properties.find(name);

	if (it == m_properties.end()) {
		return ValueWrapper();
	}

	return it->second;
}
