#include "scope.h"

#include "value.h"
#include "garbageCollector.h"


void interpreter::Scope::BindValue(std::string name, const Value& value)
{
	if (m_namedValues.find(name) == m_namedValues.end()) {
		m_namedValues[name] = Value();
	}

	SetProperty(name, value);
}

interpreter::Value interpreter::Scope::GetValue(std::string name)
{
	return GetProperty(name);
}

void interpreter::Scope::SetProperty(std::string name, Value value)
{
	Scope* cur = this;

	while (cur) {
		std::map<std::string,Value>::iterator it = cur->m_namedValues.find(name);
		if (it != cur->m_namedValues.end()) {
			volatile GarbageCollector::GCInstructionsBatch batch;

			cur->m_namedValues[name] = value;
			Value& tmp = cur->m_namedValues[name];
			if (tmp.IsManaged()) {
				tmp.SetImplicitRef(cur);
			}
			break;
		}

		if (!cur->m_parent.IsNone()) {
			cur = dynamic_cast<Scope*>(cur->m_parent.GetManagedValue());
		}
		else {
			cur = nullptr;
		}
	}
}

interpreter::Value interpreter::Scope::GetProperty(std::string name) const
{
	const Scope* cur = this;

	while (cur) {
		std::map<std::string, Value>::const_iterator it = cur->m_namedValues.find(name);
		if (it != cur->m_namedValues.end()) {
			return it->second;
		}

		if (!cur->m_parent.IsNone()) {
			cur = dynamic_cast<Scope*>(cur->m_parent.GetManagedValue());
		}
		else {
			cur = nullptr;
		}
	}
}

void interpreter::Scope::SetParentScope(interpreter::Value parentScope)
{
	volatile GarbageCollector::GCInstructionsBatch batch;

	m_parent = parentScope;
	m_parent.SetImplicitRef(this);
}