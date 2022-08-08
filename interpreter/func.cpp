#include "func.h"

#include "garbageCollector.h"
#include "scope.h"

#include "interpreter.h"
#include "calculator.h"

interpreter::Func::Func(const scripting::ISymbol& body) :
	m_body(body)
{
}

void interpreter::Func::InitFuncDefScope(const Value& funcDefScope)
{
	volatile GarbageCollector::GCInstructionsBatch batch;
	m_funcDefScope = funcDefScope;
	m_funcDefScope.SetImplicitRef(this);
}

interpreter::Value interpreter::Func::GetScopeTemplate()
{
	Value argsScope = GetArgsTemplateScope();
	Scope* scope = dynamic_cast<Scope*>(argsScope.GetManagedValue());
	scope->SetParentScope(m_funcDefScope);

	return argsScope;
}

interpreter::FuncResult interpreter::Func::Execute(Scope& scope)
{
	return FuncResult();
}
