#include "IFunc.h"
#include "scope.h"

interpreter::Value interpreter::IFunc::GetScopeTemplate()
{
	return GetArgsTemplateScope();
}

interpreter::Value interpreter::IFunc::GetArgsTemplateScope()
{
	Scope* scope = new Scope();
	Value res(*scope);
	
	for (int i = 0; i < m_paramNames.size(); ++i) {
		scope->BindValue(m_paramNames[i], Value());
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
