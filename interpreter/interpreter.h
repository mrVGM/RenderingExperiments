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
		void HandleException(const Value& exception);
		void HandleReturnInstruction(const Value& returnValue);

		const Value& GetException() const;

	private:
		bool m_breakInstruction = false;
		bool m_continueInstruction = false;
		bool m_returnInstruction = false;
		bool m_exception = false;
		Value m_exceptionValue;
		Value m_returnValue;
	};
}