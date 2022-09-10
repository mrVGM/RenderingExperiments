#include "IFunc.h"
#include "scope.h"

interpreter::Value interpreter::IFunc::GetScopeTemplate()
{
	return GetArgsTemplateScope();
}

interpreter::Value interpreter::IFunc::GetArgsTemplateScope()
{
	Value res = Scope::Create();
	Scope* scope = static_cast<Scope*>(res.GetManagedValue());
	
	for (std::list<std::string>::const_iterator i = m_paramNames.begin(); i != m_paramNames.end(); ++i) {
		scope->BindValue(*i, Value());
	}

	return res;
}

void interpreter::IFunc::SetProperty(std::string name, Value value)
{
}

interpreter::Value interpreter::IFunc::GetProperty(std::string name) const
{
	return Value();
}

interpreter::IFunc::IFunc()
{
}
