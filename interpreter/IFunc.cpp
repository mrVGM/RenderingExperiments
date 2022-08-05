#include "IFunc.h"
#include "scope.h"

interpreter::ValueWrapper interpreter::IFunc::GetScopeTemplate()
{
	return GetArgsTemplateScope();
}

interpreter::ValueWrapper interpreter::IFunc::GetArgsTemplateScope()
{
	Scope* scope = new Scope();
	
	for (int i = 0; i < m_paramNames.size(); ++i) {
		scope->BindValue(m_paramNames[i], ValueWrapper());
	}

	return ValueWrapper(*scope);
}

void interpreter::IFunc::SetProperty(std::string name, ValueWrapper value)
{
}

interpreter::ValueWrapper interpreter::IFunc::GetProperty(std::string name) const
{
	return ValueWrapper();
}
