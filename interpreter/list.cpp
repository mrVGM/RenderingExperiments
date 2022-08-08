#include "list.h"

#include "garbageCollector.h"

void interpreter::ListValue::PushValue(Value value)
{
	if (!value.IsManaged()) {
		m_list.push_back(value);
		return;
	}

	volatile GarbageCollector::GCInstructionsBatch batch;

	m_list.push_back(value);
	Value& tmp = m_list.back();

	tmp.SetImplicitRef(this);
}

interpreter::Value interpreter::ListValue::GetValueAt(int index) const
{
	if (index < 0) {
		return Value();
	}

	if (index > m_list.size()) {
		return Value();
	}

	return m_list[index];
}

void interpreter::ListValue::SetValueAt(int index, Value valueWrapper)
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

void interpreter::ListValue::SetProperty(std::string name, Value value)
{
}

interpreter::Value interpreter::ListValue::GetProperty(std::string name) const
{
	if (name == "length") {
		return Value(m_list.size());
	}
	return Value();
}
