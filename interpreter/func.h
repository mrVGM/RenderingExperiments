#pragma once

#include "IFunc.h"
#include "symbol.h"
#include "scriptingValue.h"
#include "calculator.h"

namespace interpreter
{
	struct Func : public IFunc
	{
		const scripting::ISymbol& m_body;
		ValueWrapper m_funcDefScope;

		Func(const scripting::ISymbol& body);

		void InitFuncDefScope(const ValueWrapper& funcDefScope);

		ValueWrapper GetScopeTemplate() override;
		FuncResult Execute(Scope& scope) override;
	};
}