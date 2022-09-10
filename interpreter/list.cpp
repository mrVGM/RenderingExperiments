#include "list.h"

#include "nativeFunc.h"

interpreter::Value interpreter::ListValue::GetValueAt(int index) const
{
	if (index < 0) {
		return Value();
	}

	if (index > m_list.size()) {
		return Value();
	}

	std::list<Value>::const_iterator it = m_list.begin();
	for (int i = 0; i < index; ++i) {
		++it;
	}
	return *it;
}

void interpreter::ListValue::SetValueAt(int index, Value value)
{
	if (index < 0) {
		return;
	}

	if (index > m_list.size()) {
		return;
	}

	std::list<Value>::iterator it = m_list.begin();
	for (int i = 0; i < index; ++i) {
		++it;
	}

	*it = value;
}

interpreter::Value interpreter::ListValue::Create()
{
	ListValue* lv = new ListValue();
	Value res = Value(*lv);

	lv->m_pushMethod.SetImplicitRef(*lv);
	lv->m_pushMethod = CreateNativeMethod(*lv, 1, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		ListValue* list = static_cast<ListValue*>(selfValue.GetManagedValue());

		list->m_list.push_back(Value());
		Value& back = list->m_list.back();
		back.SetImplicitRef(*list);

		Value valueToPush = scope.GetProperty("param0");
		back = valueToPush;
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
