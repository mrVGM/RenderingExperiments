#pragma once

#include "IFunc.h"
#include "symbol.h"
#include "value.h"
#include "calculator.h"

namespace interpreter
{
	struct Func : public IFunc
	{
		const scripting::ISymbol& m_body;
		Value m_funcDefScope;

		Func(const scripting::ISymbol& body);

		void InitFuncDefScope(const Value& funcDefScope);

		Value GetScopeTemplate() override;
		FuncResult Execute(Scope& scope) override;
	};
}