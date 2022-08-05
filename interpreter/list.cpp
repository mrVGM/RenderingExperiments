#include "list.h"

#include "garbageCollector.h"

void interpreter::ListValue::PushValue(ValueWrapper value)
{
	if (!value.IsManaged()) {
		m_list.push_back(value);
		return;
	}

	volatile GarbageCollector::GCInstructionsBatch batch;

	m_list.push_back(value);
	ValueWrapper& tmp = m_list.back();

	tmp.SetImplicitRef(this);
}

interpreter::ValueWrapper interpreter::ListValue::GetValueAt(int index) const
{
	if (index < 0) {
		return ValueWrapper();
	}

	if (index > m_list.size()) {
		return ValueWrapper();
	}

	return m_list[index];
}

void interpreter::ListValue::SetValueAt(int index, ValueWrapper valueWrapper)
{
	if (index < 0) {
		return;
	}

	if (index > m_list.size()) {
		return;
	}

	volatile GarbageCollector::GCInstructionsBatch batch;
	m_list[index] = valueWrapper;
	m_list[index].SetImplicitRef(this);
}

void interpreter::ListValue::SetProperty(std::string name, ValueWrapper value)
{
}

interpreter::ValueWrapper interpreter::ListValue::GetProperty(std::string name) const
{
	if (name == "length") {
		return ValueWrapper(m_list.size());
	}
	return ValueWrapper();
}
