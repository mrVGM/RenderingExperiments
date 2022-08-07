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
		ValueWrapper m_initialScope;
		ValueWrapper m_scope;

		InterpreterState m_state = InterpreterState::Pending;
		std::stack<Calculator*> m_programStack;


		Interpreter(const ValueWrapper& scope);
		~Interpreter();

		void FreeUpResources();

		Scope& GetCurrentScope();
		void PushScope();
		void PopScope();

		void Push(Calculator* calculator);

		void Calculate(scripting::ISymbol* symbol);

		void PrepareCalculation(scripting::ISymbol* symbol);
		void CalcutateStep();

		void HandleContinueInstruction();
		void HandleBreakInstruction();
		void HandleReturnInstruction(const ValueWrapper& returnValue);

	private:
		bool m_breakInstruction = false;
		bool m_continueInstruction = false;
		bool m_returnInstruction = false;
		ValueWrapper m_returnValue;
	};
}