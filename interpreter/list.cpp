#include "list.h"

#include "nativeFunc.h"

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

interpreter::Value interpreter::ListValue::Create()
{
	ListValue* lv = new ListValue();
	Value res = Value(*lv);

	lv->m_pushMethod.SetImplicitRef(*lv);
	lv->m_pushMethod = CreateNativeMethod(*lv, 1, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		ListValue* list = static_cast<ListValue*>(selfValue.GetManagedValue());

		Value valueToPush = scope.GetProperty("param0");
		list->m_list.push_back(valueToPush);
		return Value();
	});

	return res;
}

interpreter::ListValue::ListValue()
{
}

void interpreter::ListValue::SetProperty(std::string name, Value value)
{
}

interpreter::Value interpreter::ListValue::GetProperty(std::string name) const
{
	if (name == "length") {
		return Value(m_list.size());
	}

	if (name == "push") {
		return m_pushMethod;
	}
	return Value();
}
