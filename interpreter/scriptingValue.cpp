#include "scriptingValue.h"
#include "garbageCollector.h"

interpreter::ValueWrapper::ValueWrapper()
{
}

interpreter::ValueWrapper::ValueWrapper(double num)
{
	m_type = ScriptingValueType::Number;
	m_number = num;
}

interpreter::ValueWrapper::ValueWrapper(std::string str)
{
	m_type = ScriptingValueType::String;
	m_string = str;
}

void interpreter::ValueWrapper::Copy(const ValueWrapper& other)
{
	m_type = other.m_type;

	m_value = other.m_value;
	m_number = other.m_number;
	m_string = other.m_string;
}

interpreter::ValueWrapper::ValueWrapper(IManagedValue& value) :
	ValueWrapper(value, ScriptingValueType::Object)
{
}

interpreter::ValueWrapper::ValueWrapper(IManagedValue& value, ScriptingValueType managedValueType)
{
	m_type = managedValueType;
	m_value = &value;

	GarbageCollector::GetInstance().AddExplicitRef(&value);
}

interpreter::ValueWrapper::ValueWrapper(const ValueWrapper& other)
{
	Copy(other);
	if (IsManaged()) {
		GarbageCollector::GetInstance().AddExplicitRef(m_value);
	}
}

interpreter::ValueWrapper& interpreter::ValueWrapper::operator=(const ValueWrapper& other)
{
	volatile GarbageCollector::GCInstructionsBatch batch;
	if (IsManaged()) {
		if (m_explicitRef) {
			GarbageCollector::GetInstance().RemoveExplicitRef(m_value);
		}

		if (m_implicitRef) {
			GarbageCollector::GetInstance().RemoveImplicitRef(m_value, m_implicitRef);
		}
	}

	Copy(other);
	if (IsManaged()) {
		GarbageCollector::GetInstance().AddExplicitRef(m_value);
	}
	return *this;
}

interpreter::ValueWrapper::~ValueWrapper()
{
	if (!IsManaged()) {
		return;
	}

	if (m_explicitRef) {
		GarbageCollector::GetInstance().RemoveExplicitRef(m_value);
	}
	
	if (m_implicitRef) {
		GarbageCollector::GetInstance().RemoveImplicitRef(m_value, m_implicitRef);
	}
}

bool interpreter::ValueWrapper::Equals(const ValueWrapper& other) const
{
	return m_type == other.m_type;
}

bool interpreter::ValueWrapper::IsManaged() const
{
	return GetType() == ScriptingValueType::Object || GetType() == ScriptingValueType::List || GetType() == ScriptingValueType::Func;
}

bool interpreter::ValueWrapper::IsNone() const
{
	return GetType() == ScriptingValueType::None;
}

void interpreter::ValueWrapper::SetImplicitRef(IManagedValue* implicitRef)
{
	if (!IsManaged()) {
		return;
	}

	GarbageCollector::GetInstance().AddImplicitRef(m_value, implicitRef);
	GarbageCollector::GetInstance().RemoveExplicitRef(m_value);
}

double interpreter::ValueWrapper::GetNum() const
{
	return m_number;
}

std::string interpreter::ValueWrapper::GetString() const
{
	return m_string;
}

interpreter::IManagedValue* interpreter::ValueWrapper::GetManagedValue() const
{
	return m_value;
}

interpreter::ScriptingValueType interpreter::ValueWrapper::GetType() const
{
	return m_type;
}

void interpreter::ValueWrapper::SetProperty(std::string name, ValueWrapper value)
{
	m_value->SetProperty(name, value);
}

interpreter::ValueWrapper interpreter::ValueWrapper::GetProperty(std::string name)
{
	return m_value->GetProperty(name);
}
