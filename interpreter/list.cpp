#include "list.h"

#include "garbageCollector.h"

void interpreter::ListValue::PushValue(Value value)
{
	m_list.push_back(Value());
	Value& val = m_list.back();

	val.SetImplicitRef(*this);
	val = value;
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

	m_list[index] = valueWrapper;
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
