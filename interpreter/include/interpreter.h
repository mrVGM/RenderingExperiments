#pragma once

#include "symbol.h"
#include <scriptingValue.h>

#include <stack>
#include <vector>

namespace interpreter
{
	struct Calculator;
	struct Scope;

	enum InterpreterState
	{
		Pending,
		Done,
		Failed,
	};

	struct Interpreter
	{
		ValueWrapper m_scope;

		InterpreterState m_state;
		std::stack<Calculator*> m_programStack;

		Interpreter();
		~Interpreter();

		Scope& GetCurrentScope();
		void PushScope();
		void PopScope();

		void Push(Calculator* calculator);

		void Calculate(scripting::ISymbol* symbol);

		void PrepareCalculation(scripting::ISymbol* symbol);
		void CalcutateStep();
	};
}