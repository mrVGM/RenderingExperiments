#include "value.h"

#include "garbageCollector.h"
#include "list.h"

#include <sstream>

interpreter::Value::Value()
{
}

interpreter::Value::Value(double num)
{
	m_type = ScriptingValueType::Number;
	m_number = num;
}

interpreter::Value::Value(std::string str)
{
	m_type = ScriptingValueType::String;
	m_string = str;
}

void interpreter::Value::Copy(const Value& other)
{
	volatile GarbageCollector::GCInstructionsBatch batch;
	GarbageCollector& gc = GarbageCollector::GetInstance();
	
	if (m_type == ScriptingValueType::Object) {
		if (other.m_type == ScriptingValueType::Object && m_value == other.m_value) {
			return;
		}

		if (m_explicitRef) {
			gc.RemoveExplicitRef(m_value);
		}
		if (m_outerObject) {
			gc.RemoveImplicitRef(m_value, m_outerObject);
		}
	}

	if (other.m_type == ScriptingValueType::Object) {
		m_value = other.m_value;
		if (m_explicitRef) {
			gc.AddExplicitRef(m_value);
		}
		if (m_outerObject) {
			gc.AddImplicitRef(m_value, m_outerObject);
		}
	}

	m_type = other.m_type;
	m_number = other.m_number;
	m_string = other.m_string;
}

interpreter::Value::Value(IManagedValue& value)
{
	m_type = ScriptingValueType::Object;
	m_value = &value;

	GarbageCollector::GetInstance().AddExplicitRef(&value);
}

interpreter::Value::Value(const Value& other)
{
	Copy(other);
}

interpreter::Value& interpreter::Value::operator=(const Value& other)
{
	Copy(other);
	return *this;
}

interpreter::Value::~Value()
{
	if (!IsManaged()) {
		return;
	}

	if (m_explicitRef) {
		m_explicitRef = false;
		GarbageCollector::GetInstance().RemoveExplicitRef(m_value);
	}

	if (m_outerObject) {
		IManagedValue* ref = m_outerObject;
		m_outerObject = nullptr;
		GarbageCollector::GetInstance().RemoveImplicitRef(m_value, ref);
	}
}

bool interpreter::Value::Equals(const Value& other) const
{
	if (m_type != other.m_type) {
		return false;
	}

	switch (m_type) {
	case ScriptingValueType::None:
		return true;
	case ScriptingValueType::Number:
		return GetNum() == other.GetNum();
	case ScriptingValueType::String:
		return GetString() == other.GetString();
	case ScriptingValueType::Object:
		return GetManagedValue() == other.GetManagedValue();
	}

	return false;
}

bool interpreter::Value::IsTrue() const
{
	if (GetType() == ScriptingValueType::Number) {
		return GetNum();
	}
	return false;
}

interpreter::Value interpreter::Value::Plus(const Value& v1, const Value& v2)
{
	if (v1.GetType() == ScriptingValueType::Number && v2.GetType() == ScriptingValueType::Number) {
		return Value(v1.GetNum() + v2.GetNum());
	}

	if (v1.GetType() == ScriptingValueType::String && v2.GetType() == ScriptingValueType::String) {
		return Value(v1.GetString() + v2.GetString());
	}
	return Value();
}

interpreter::Value interpreter::Value::Minus(const Value& v1, const Value& v2)
{
	if (v1.GetType() == ScriptingValueType::Number && v2.GetType() == ScriptingValueType::Number) {
		return Value(v1.GetNum() - v2.GetNum());
	}

	return Value();
}

interpreter::Value interpreter::Value::Multiply(const Value& v1, const Value& v2)
{
	if (v1.GetType() == ScriptingValueType::Number && v2.GetType() == ScriptingValueType::Number) {
		return Value(v1.GetNum() * v2.GetNum());
	}

	return Value();
}

interpreter::Value interpreter::Value::Divide(const Value& v1, const Value& v2)
{
	if (v1.GetType() == ScriptingValueType::Number && v2.GetType() == ScriptingValueType::Number) {
		if (v2.GetNum() == 0.0) {
			return Value();
		}
		return Value(v1.GetNum() / v2.GetNum());
	}

	return Value();
}

interpreter::Value interpreter::Value::Quotient(const Value& v1, const Value& v2)
{
	if (v1.GetType() == ScriptingValueType::Number && v2.GetType() == ScriptingValueType::Number) {
		if (v2.GetNum() == 0.0) {
			return Value();
		}

		double a = v1.GetNum();
		double b = v2.GetNum();

		while (a > b) {
			a -= b;
		}

		while (a < b) {
			a += b;
		}

		return Value(a - b);
	}

	return Value();
}

interpreter::Value interpreter::Value::Negate(const Value& v1)
{
	if (v1.GetType() == ScriptingValueType::Number) {
		return Value(-v1.GetNum());
	}

	return Value();
}

interpreter::Value interpreter::Value::Equal(const Value& v1, const Value& v2)
{
	if (v1.Equals(v2)) {
		return Value(1);
	}
	else {
		return Value(0);
	}
	return Value();
}

interpreter::Value interpreter::Value::Less(const Value& v1, const Value& v2)
{
	if (v1.GetType() == ScriptingValueType::Number && v2.GetType() == ScriptingValueType::Number) {
		return Value(v1.GetNum() < v2.GetNum());
	}
	return Value();
}

interpreter::Value interpreter::Value::LessOrEqual(const Value& v1, const Value& v2)
{
	if (v1.GetType() == ScriptingValueType::Number && v2.GetType() == ScriptingValueType::Number) {
		return Value(v1.GetNum() <= v2.GetNum());
	}
	return Value();
}

interpreter::Value interpreter::Value::Greater(const Value& v1, const Value& v2)
{
	if (v1.GetType() == ScriptingValueType::Number && v2.GetType() == ScriptingValueType::Number) {
		return Value(v1.GetNum() > v2.GetNum());
	}
	return Value();
}

interpreter::Value interpreter::Value::GreaterOrEqual(const Value& v1, const Value& v2)
{
	if (v1.GetType() == ScriptingValueType::Number && v2.GetType() == ScriptingValueType::Number) {
		return Value(v1.GetNum() >= v2.GetNum());
	}
	return Value();
}

interpreter::Value interpreter::Value::And(const Value& v1, const Value& v2)
{
	if (v1.GetType() == ScriptingValueType::Number && v2.GetType() == ScriptingValueType::Number) {
		return Value(v1.GetNum() && v2.GetNum());
	}
	return Value();
}

interpreter::Value interpreter::Value::Or(const Value& v1, const Value& v2)
{
	if (v1.GetType() == ScriptingValueType::Number && v2.GetType() == ScriptingValueType::Number) {
		return Value(v1.GetNum() || v2.GetNum());
	}
	return Value();
}

interpreter::Value interpreter::Value::Not(const Value& v1)
{
	if (v1.GetType() == ScriptingValueType::Number) {
		return Value(!v1.GetNum());
	}
	return Value();
}

std::string interpreter::Value::ToString() const
{
	std::stringstream ss;

	switch (m_type) {
	case ScriptingValueType::None:
		ss << "<None>";
		break;
	case ScriptingValueType::Number:
		ss << GetNum();
		break;
	case ScriptingValueType::String:
		ss << "\"" << GetString() << "\"";
		break;
	case ScriptingValueType::Object:
		ss << "<Object>";
		break;
	}

	return ss.str();
}

void interpreter::Value::ToList(std::vector<Value>& list) const
{
	if (GetType() != ScriptingValueType::Object) {
		list.clear();
		return;
	}
	ListValue* lv = dynamic_cast<ListValue*>(GetManagedValue());
	if (!lv) {
		list.clear();
		return;
	}

	list = lv->m_list;
}

bool interpreter::Value::IsManaged() const
{
	return GetType() == ScriptingValueType::Object;
}

bool interpreter::Value::IsNone() const
{
	return GetType() == ScriptingValueType::None;
}

void interpreter::Value::SetImplicitRef(IManagedValue& outerObject)
{
	if (m_value != nullptr) {
		bool t = true;
	}

	m_explicitRef = false;
	m_outerObject = &outerObject;
}

double interpreter::Value::GetNum() const
{
	return m_number;
}

std::string interpreter::Value::GetString() const
{
	return m_string;
}

interpreter::IManagedValue* interpreter::Value::GetManagedValue() const
{
	return m_value;
}

interpreter::ScriptingValueType interpreter::Value::GetType() const
{
	return m_type;
}

void interpreter::Value::SetProperty(std::string name, Value value)
{
	m_value->SetProperty(name, value);
}

interpreter::Value interpreter::Value::GetProperty(std::string name)
{
	return m_value->GetProperty(name);
}
