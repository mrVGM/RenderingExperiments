#include "interpreter.h"

#include "symbol.h"
#include "calculator.h"
#include "scope.h"
#include "IFunc.h"
#include "value.h"

#include <iostream>

interpreter::Interpreter::Interpreter(const Value& scope) :
	m_initialScope(scope)
{
}

interpreter::Interpreter::~Interpreter()
{
	FreeUpResources();
}

void interpreter::Interpreter::FreeUpResources()
{
	Calculator* top = nullptr;
	while (!m_programStack.empty()) {
		top = m_programStack.top();
		m_programStack.pop();
		top->FreeUpResources();
	}

	if (top) {
		delete top;
	}

	m_returnValue = Value();
	m_scope = Value();
}

interpreter::Scope& interpreter::Interpreter::GetCurrentScope()
{
	Scope* tmp = dynamic_cast<Scope*>(m_scope.GetManagedValue());
	return *tmp;
}

void interpreter::Interpreter::Push(Calculator* calculator)
{
	m_programStack.push(calculator);
}

void interpreter::Interpreter::Calculate(scripting::ISymbol* symbol)
{
	PrepareCalculation(symbol);
	while (m_state == InterpreterState::Pending) {
		CalcutateStep();
	}
}

void interpreter::Interpreter::PrepareCalculation(scripting::ISymbol* symbol)
{
	m_scope = m_initialScope;
	m_state = InterpreterState::Pending;

	InitialCalc* initialCalc = new InitialCalc();
	Calculator* calc = new Calculator(*this, *symbol, *initialCalc);

	Push(calc);
}

void interpreter::Interpreter::CalcutateStep()
{
	if (m_programStack.empty()) {
		if (m_breakInstruction || m_continueInstruction || m_returnInstruction || m_exception) {
			m_state = InterpreterState::Failed;
		}
		else {
			m_state = InterpreterState::Done;
		}
		return;
	}

	Calculator* top = m_programStack.top();

	if (m_exception) {
		TryCatchCalc* tryCatchCalc = dynamic_cast<TryCatchCalc*>(&top->m_calculator);
		if (tryCatchCalc) {
			tryCatchCalc->m_exception = true;
			tryCatchCalc->m_exceptionValue = m_exceptionValue;
			m_exception = false;
			m_exceptionValue = Value();
			return;
		}

		m_programStack.pop();
		top->FreeUpResources();

		if (m_programStack.empty()) {
			delete top;
		}

		return;
	}

	if (m_breakInstruction) {
		WhileStatementCalc* whileCalc = dynamic_cast<WhileStatementCalc*>(&top->m_calculator);
		if (whileCalc) {
			whileCalc->m_breakInstruction = true;
			m_breakInstruction = false;
			return;
		}

		m_programStack.pop();
		top->FreeUpResources();

		if (m_programStack.empty()) {
			delete top;
		}

		return;
	}

	if (m_continueInstruction) {
		WhileStatementCalc* whileCalc = dynamic_cast<WhileStatementCalc*>(&top->m_calculator);
		if (whileCalc) {
			whileCalc->m_continueInstruction = true;
			m_continueInstruction = false;
			return;
		}

		m_programStack.pop();
		top->FreeUpResources();

		if (m_programStack.empty()) {
			delete top;
		}

		return;
	}

	if (m_returnInstruction) {
		FuncCallCalc* funcCallCalc = dynamic_cast<FuncCallCalc*>(&top->m_calculator);
		if (funcCallCalc) {
			funcCallCalc->m_returnInstruction = true;
			top->m_calculation.m_value = m_returnValue;
			m_returnValue = Value();
			m_returnInstruction = false;
			return;
		}

		m_programStack.pop();
		top->FreeUpResources();

		if (m_programStack.empty()) {
			delete top;
		}

		return;
	}

	top->Calculate();

	if (top->m_calculation.m_state != Calculation::CalculationState::Pending) {
		top->FreeUpResources();
	}

	if (top->m_calculation.m_state == Calculation::Failed) {
		HandleException(top->m_calculation.m_value);
		return;
	}

	if (top->m_calculation.m_state == Calculation::Done) {
		m_programStack.pop();

		if (m_programStack.empty()) {
			delete top;
		}
	}
}

void interpreter::Interpreter::HandleContinueInstruction()
{
	m_continueInstruction = true;
}

void interpreter::Interpreter::HandleBreakInstruction()
{
	m_breakInstruction = true;
}

void interpreter::Interpreter::HandleException(const Value& exception)
{
	m_exception = true;
	m_exceptionValue = exception;
}

void interpreter::Interpreter::HandleReturnInstruction(const Value& returnValue)
{
	m_returnInstruction = true;
	m_returnValue = returnValue;
}

const interpreter::Value& interpreter::Interpreter::GetException() const
{
	return m_exceptionValue;
}

void interpreter::Interpreter::PushScope()
{
	Value newScopeWrapped = Scope::Create();
	Scope* newScope = static_cast<Scope*>(newScopeWrapped.GetManagedValue());

	newScope->SetParentScope(m_scope);

	m_scope = newScopeWrapped;
}

void interpreter::Interpreter::PopScope()
{
	Scope& scope = GetCurrentScope();
	m_scope = scope.m_parent;
}