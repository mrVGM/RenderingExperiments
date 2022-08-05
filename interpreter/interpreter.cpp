#include "interpreter.h"

#include "symbol.h"
#include "calculator.h"
#include "scope.h"
#include "IFunc.h"
#include "scriptingValue.h"

#include <iostream>

struct PrintFunc : public interpreter::IFunc
{
	PrintFunc()
	{
		m_paramNames.push_back("str");
	}

	interpreter::FuncResult Execute(interpreter::Scope& scope) override
	{
		interpreter::ValueWrapper val = scope.GetValue(m_paramNames[0]);
		std::cout << val.GetString() << std::endl;
		interpreter::FuncResult res;
		res.m_state = interpreter::FuncResult::Finished;
		return res;
	}
};


interpreter::Interpreter::Interpreter()
{
	Scope* scope = new Scope();
	m_scope = ValueWrapper(*scope);

	PrintFunc* printFunc = new PrintFunc();
	ValueWrapper print(*printFunc);

	scope->BindValue("print", print);
}

interpreter::Interpreter::~Interpreter()
{
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
	m_state = InterpreterState::Pending;

	InitialCalc* initialCalc = new InitialCalc();
	Calculator* calc = new Calculator(*this, *symbol, *initialCalc);

	Push(calc);
}

void interpreter::Interpreter::CalcutateStep()
{
	if (m_programStack.empty()) {
		m_state = InterpreterState::Done;
		return;
	}

	Calculator* top = m_programStack.top();
	top->Calculate();

	if (top->m_calculation.m_state != Calculation::CalculationState::Pending) {
		top->FreeUpResources();
	}

	if (top->m_calculation.m_state == Calculation::Failed) {
		m_state = InterpreterState::Failed;
		return;
	}

	if (top->m_calculation.m_state == Calculation::Done) {
		m_programStack.pop();

		if (m_programStack.empty()) {
			delete top;
		}
	}
}

void interpreter::Interpreter::PushScope()
{
	Scope* newScope = new Scope();
	ValueWrapper newScopeWrapped(*newScope);
	newScope->SetParentScope(m_scope);

	m_scope = newScopeWrapped;
}

void interpreter::Interpreter::PopScope()
{
	Scope& scope = GetCurrentScope();
	m_scope = scope.m_parent;
}