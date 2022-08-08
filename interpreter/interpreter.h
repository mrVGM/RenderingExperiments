#pragma once

#include "symbol.h"
#include "value.h"

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
		Value m_initialScope;
		Value m_scope;

		InterpreterState m_state = InterpreterState::Pending;
		std::stack<Calculator*> m_programStack;


		Interpreter(const Value& scope);
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
		void HandleReturnInstruction(const Value& returnValue);

	private:
		bool m_breakInstruction = false;
		bool m_continueInstruction = false;
		bool m_returnInstruction = false;
		Value m_returnValue;
	};
}