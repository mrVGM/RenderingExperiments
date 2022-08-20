#include "calculator.h"

#include "interpreter.h"
#include "scope.h"
#include "list.h"
#include "object.h"
#include "list.h"
#include "IFunc.h"
#include "func.h"

#include <cstring>

namespace symbolUtils
{
	bool MatchChildren(const scripting::ISymbol* symbol, const char* names[], const scripting::CompositeSymbol*& compositeSymbol)
	{
		compositeSymbol = dynamic_cast<const scripting::CompositeSymbol*>(symbol);

		int size = 0;
		while (names[size]) {
			++size;
		}

		if (size != compositeSymbol->m_childSymbols.size()) {
			return false;
		}

		for (int i = 0; i < size; ++i) {
			if (strcmp(names[i], compositeSymbol->m_childSymbols[i]->m_name.c_str()) != 0) {
				return false;
			}
		}

		return true;
	}
}

interpreter::Calculator::Calculator(Interpreter& interpreter, const scripting::ISymbol& symbol, ICalculator& m_calculator) :
	m_interpreter(interpreter),
	m_symbol(symbol),
	m_calculator(m_calculator)
{
}

interpreter::Calculator::~Calculator()
{
	delete& m_calculator;
}

void interpreter::Calculator::Calculate()
{
	m_calculator.Calculate(*this);
}

void interpreter::Calculator::FreeUpResources()
{
	m_calculator.FreeUpResouces();
}


void interpreter::InitialCalc::Calculate(Calculator& calculator)
{
	if (!m_instructionsCalc) {
		scripting::CompositeSymbol& cs = (scripting::CompositeSymbol&)calculator.m_symbol;
		InstructionsCalc* calc = new InstructionsCalc();
		m_instructionsCalc = new Calculator(calculator.m_interpreter, *cs.m_childSymbols[0], *calc);
		calculator.m_interpreter.Push(m_instructionsCalc);
	}

	calculator.m_calculation = m_instructionsCalc->m_calculation;
}

interpreter::InitialCalc::~InitialCalc()
{
}

void interpreter::InitialCalc::FreeUpResouces()
{
	if (m_instructionsCalc) {
		delete m_instructionsCalc;
	}
	m_instructionsCalc = nullptr;
}

void interpreter::InstructionsCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;

	const char* multiInstructions[] = { "Instructions", "Instruction", 0};
	const char* singleInstruction[] = { "Instruction", 0};

	if (symbolUtils::MatchChildren(&calculator.m_symbol, singleInstruction, cs)) {

		if (!m_singleInstruction) {
			SingleInstructionCalc* singleInst = new SingleInstructionCalc();
			m_singleInstruction = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *singleInst);
			calculator.m_interpreter.Push(m_singleInstruction);
		}

		calculator.m_calculation = m_singleInstruction->m_calculation;
		return;
	}

	if (!m_manyInstructions) {
		InstructionsCalc* many = new InstructionsCalc();
		m_manyInstructions = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *many);

		SingleInstructionCalc* single = new SingleInstructionCalc();
		m_singleInstruction = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[1], *single);

		calculator.m_interpreter.Push(m_singleInstruction);
		calculator.m_interpreter.Push(m_manyInstructions);
	}

	if (m_manyInstructions->m_calculation.m_state != Calculation::CalculationState::Done) {
		calculator.m_calculation = m_manyInstructions->m_calculation;
		return;
	}

	if (m_singleInstruction->m_calculation.m_state != Calculation::CalculationState::Done) {
		calculator.m_calculation = m_manyInstructions->m_calculation;
		return;
	}
	
	calculator.m_calculation = m_singleInstruction->m_calculation;
}

interpreter::InstructionsCalc::~InstructionsCalc()
{
}

void interpreter::InstructionsCalc::FreeUpResouces()
{
	if (m_manyInstructions) {
		delete m_manyInstructions;
	}
	if (m_singleInstruction) {
		delete m_singleInstruction;
	}
	m_manyInstructions = nullptr;
	m_singleInstruction = nullptr;
}

void interpreter::SingleInstructionCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;

	const char* assignment[] = { "Assignment", 0 };
	const char* ifStatement[] = { "IfStatement", 0 };
	const char* whileStatement[] = { "WhileStatement", 0 };
	const char* tryCatch[] = { "TryCatch", 0 };
	const char* varDef[] = { "VarDef", 0 };
	const char* funcCall[] = { "FuncCall", ";", 0 };
	const char* breakStatement[] = { "break", ";", 0 };
	const char* continueStatement[] = { "continue", ";", 0 };

	const char* returnStatement[] = { "return", ";", 0 };
	const char* returnValueStatement[] = { "return", "Expression", ";", 0 };

	if (symbolUtils::MatchChildren(&calculator.m_symbol,assignment, cs)) {
		if (!m_calc) {
			AssignmentCalc* assignmentCalc = new AssignmentCalc();
			m_calc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *assignmentCalc);
			calculator.m_interpreter.Push(m_calc);
		}
		calculator.m_calculation = m_calc->m_calculation;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, ifStatement, cs)) {
		if (!m_calc) {
			IfStatementCalc* ifStatementCalc = new IfStatementCalc();
			m_calc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *ifStatementCalc);
			calculator.m_interpreter.Push(m_calc);
		}
		calculator.m_calculation = m_calc->m_calculation;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, whileStatement, cs)) {
		if (!m_calc) {
			WhileStatementCalc* whileStatementCalc = new WhileStatementCalc();
			m_calc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *whileStatementCalc);
			calculator.m_interpreter.Push(m_calc);
		}
		calculator.m_calculation = m_calc->m_calculation;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, tryCatch, cs)) {
		if (!m_calc) {
			TryCatchCalc* tryCatchCalc = new TryCatchCalc();
			m_calc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *tryCatchCalc);
			calculator.m_interpreter.Push(m_calc);
		}
		calculator.m_calculation = m_calc->m_calculation;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, varDef, cs)) {
		if (!m_calc) {
			VarDefCalc* varDefCalc = new VarDefCalc();
			m_calc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *varDefCalc);
			calculator.m_interpreter.Push(m_calc);
		}
		calculator.m_calculation = m_calc->m_calculation;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, funcCall, cs)) {
		if (!m_calc) {
			FuncCallCalc* funcCall = new FuncCallCalc();
			m_calc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *funcCall);
			calculator.m_interpreter.Push(m_calc);
		}
		calculator.m_calculation = m_calc->m_calculation;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, breakStatement, cs)) {
		calculator.m_interpreter.HandleBreakInstruction();
		calculator.m_calculation.m_state = Calculation::Done;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, breakStatement, cs)) {
		calculator.m_interpreter.HandleBreakInstruction();
		calculator.m_calculation.m_state = Calculation::Done;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, continueStatement, cs)) {
		calculator.m_interpreter.HandleContinueInstruction();
		calculator.m_calculation.m_state = Calculation::Done;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, returnStatement, cs)) {
		calculator.m_interpreter.HandleReturnInstruction(Value());
		calculator.m_calculation.m_state = Calculation::Done;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, returnValueStatement, cs)) {
		if (!m_calc) {
			ExpressionCalc* exprCalc = new ExpressionCalc();
			m_calc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[1], *exprCalc);
			calculator.m_interpreter.Push(m_calc);
		}

		if (m_calc->m_calculation.m_state == Calculation::CalculationState::Pending) {
			return;
		}

		calculator.m_interpreter.HandleReturnInstruction(m_calc->m_calculation.m_value);
		calculator.m_calculation.m_state = Calculation::Done;
		return;
	}

	calculator.m_calculation = Calculation::GetFailed();
}

interpreter::SingleInstructionCalc::~SingleInstructionCalc()
{
}

void interpreter::SingleInstructionCalc::FreeUpResouces()
{
	if (m_calc) {
		delete m_calc;
	}
	m_calc = nullptr;
}

interpreter::Calculation interpreter::Calculation::GetFailed()
{
	Calculation res;
	res.m_state = CalculationState::Failed;
	return res;
}

void interpreter::AssignmentCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;

	const char* assignment[] = { "RawValue", "=", "Expression", ";", 0 };

	if (!symbolUtils::MatchChildren(&calculator.m_symbol, assignment, cs)) {
		calculator.m_calculation = Calculation::GetFailed();
		return;
	}

	if (!m_expressionCalc) {
		ExpressionCalc* expressionCalc = new ExpressionCalc();
		m_expressionCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[2], *expressionCalc);
		calculator.m_interpreter.Push(m_expressionCalc);
	}

	if (!m_lValueCalc) {
		LValueCalc* lValueCalc = new LValueCalc();
		m_lValueCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *lValueCalc);
		calculator.m_interpreter.Push(m_lValueCalc);
	}

	if (m_lValueCalc->m_calculation.m_state == Calculation::CalculationState::Pending) {
		return;
	}

	if (m_expressionCalc->m_calculation.m_state == Calculation::CalculationState::Pending) {
		return;
	}

	LValueCalc& lValueCalc = (LValueCalc&) m_lValueCalc->m_calculator;
	if (lValueCalc.m_outerObject.GetType() == ScriptingValueType::None && lValueCalc.m_prop.GetType() == ScriptingValueType::String) {
		calculator.m_interpreter.GetCurrentScope().SetProperty(lValueCalc.m_prop.GetString(), m_expressionCalc->m_calculation.m_value);
		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		return;
	}

	if (lValueCalc.m_outerObject.GetType() == ScriptingValueType::Object) {
		if (lValueCalc.m_prop.GetType() == ScriptingValueType::String) {
			lValueCalc.m_outerObject.SetProperty(lValueCalc.m_prop.GetString(), m_expressionCalc->m_calculation.m_value);
			calculator.m_calculation.m_state = Calculation::CalculationState::Done;
			return;
		}

		if (lValueCalc.m_prop.GetType() == ScriptingValueType::Number) {
			ListValue* list = dynamic_cast<ListValue*>(lValueCalc.m_outerObject.GetManagedValue());
			if (!list) {
				calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
				return;
			}

			list->SetValueAt(lValueCalc.m_prop.GetNum(), m_expressionCalc->m_calculation.m_value);
			calculator.m_calculation.m_state = Calculation::CalculationState::Done;
			return;
		}
	}

	calculator.m_calculation = Calculation::GetFailed();
}

interpreter::AssignmentCalc::~AssignmentCalc()
{
}

void interpreter::AssignmentCalc::FreeUpResouces()
{
	if (m_lValueCalc) {
		delete m_lValueCalc;
	}
	if (m_expressionCalc) {
		delete m_expressionCalc;
	}

	m_lValueCalc = nullptr;
	m_expressionCalc = nullptr;
}

void interpreter::ExpressionCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;

	const char* arithmeticExpression[] = { "ArithmeticExpression", 0 };
	const char* binaryExpression[] = { "BinaryExpression", 0 };

	if (symbolUtils::MatchChildren(&calculator.m_symbol, arithmeticExpression, cs)) {
		if (!m_calc) {
			ArithmethicExpressionCalc* arithmeticCalc = new ArithmethicExpressionCalc();
			m_calc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *arithmeticCalc);
			calculator.m_interpreter.Push(m_calc);
		}

		calculator.m_calculation = m_calc->m_calculation;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, binaryExpression, cs)) {
		if (!m_calc) {
			BinaryExpressionCalc* binaryExpressionCalc = new BinaryExpressionCalc();
			m_calc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *binaryExpressionCalc);
			calculator.m_interpreter.Push(m_calc);
		}

		calculator.m_calculation = m_calc->m_calculation;
		return;
	}

	calculator.m_calculation = Calculation::GetFailed();
}

interpreter::ExpressionCalc::~ExpressionCalc()
{
}

void interpreter::ExpressionCalc::FreeUpResouces()
{
	if (m_calc) {
		delete m_calc;
	}
	m_calc = nullptr;
}

void interpreter::ArithmethicExpressionCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;

	const char* sumExpr[] = { "Sum", 0 };

	if (symbolUtils::MatchChildren(&calculator.m_symbol, sumExpr, cs)) {
		if (!m_sumCalc) {
			SumCalc* sumCalc = new SumCalc();
			m_sumCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *sumCalc);
			calculator.m_interpreter.Push(m_sumCalc);
		}

		calculator.m_calculation = m_sumCalc->m_calculation;
		return;
	}

	calculator.m_calculation = Calculation::GetFailed();
}

interpreter::ArithmethicExpressionCalc::~ArithmethicExpressionCalc()
{
}

void interpreter::ArithmethicExpressionCalc::FreeUpResouces()
{
	if (m_sumCalc) {
		delete m_sumCalc;
	}
	m_sumCalc = nullptr;
}

void interpreter::SumCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;

	const char* productExpr[] = { "Product", 0 };
	const char* plusExpr[] = { "Sum", "+", "Product", 0};
	const char* minusExpr[] = { "Sum", "-", "Product", 0};

	if (symbolUtils::MatchChildren(&calculator.m_symbol, productExpr, cs)) {
		if (!m_productCalc) {
			ProductCalc* productCalc = new ProductCalc();
			m_productCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *productCalc);
			calculator.m_interpreter.Push(m_productCalc);
		}

		calculator.m_calculation = m_productCalc->m_calculation;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, plusExpr, cs)) {
		if (!m_productCalc) {
			ProductCalc* productCalc = new ProductCalc();
			m_productCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[2], *productCalc);
			calculator.m_interpreter.Push(m_productCalc);
		}

		if (!m_sumCalc) {
			SumCalc* sumCalc = new SumCalc();
			m_sumCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *sumCalc);
			calculator.m_interpreter.Push(m_sumCalc);
		}

		if (m_sumCalc->m_calculation.m_state != Calculation::CalculationState::Done ||
			m_productCalc->m_calculation.m_state != Calculation::CalculationState::Done) {
			return;
		}

		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		calculator.m_calculation.m_value = Value::Plus(m_sumCalc->m_calculation.m_value, m_productCalc->m_calculation.m_value);
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, minusExpr, cs)) {
		if (!m_productCalc) {
			ProductCalc* productCalc = new ProductCalc();
			m_productCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[2], *productCalc);
			calculator.m_interpreter.Push(m_productCalc);
		}

		if (!m_sumCalc) {
			SumCalc* sumCalc = new SumCalc();
			m_sumCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *sumCalc);
			calculator.m_interpreter.Push(m_sumCalc);
		}

		if (m_sumCalc->m_calculation.m_state != Calculation::CalculationState::Done ||
			m_productCalc->m_calculation.m_state != Calculation::CalculationState::Done) {
			return;
		}

		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		calculator.m_calculation.m_value = Value::Minus(m_sumCalc->m_calculation.m_value, m_productCalc->m_calculation.m_value);
		return;
	}

	calculator.m_calculation = Calculation::GetFailed();
}

void interpreter::SumCalc::FreeUpResouces()
{
	if (m_sumCalc) {
		delete m_sumCalc;
	}
	if (m_productCalc) {
		delete m_productCalc;
	}

	m_sumCalc = nullptr;
	m_productCalc = nullptr;
}

interpreter::SumCalc::~SumCalc()
{
}

void interpreter::ProductCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;

	const char* numberExpr[] = { "NumberValue", 0 };
	const char* multiplyExpr[] = { "Product", "*", "NumberValue", 0 };
	const char* divideExpr[] = { "Product", "/", "NumberValue", 0 };
	const char* quotientExpr[] = { "Product", "%", "NumberValue", 0 };

	if (symbolUtils::MatchChildren(&calculator.m_symbol, numberExpr, cs)) {
		if (!m_numberCalc) {
			NumberValueCalc* numberCalc = new NumberValueCalc();
			m_numberCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *numberCalc);
			calculator.m_interpreter.Push(m_numberCalc);
		}

		calculator.m_calculation = m_numberCalc->m_calculation;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, multiplyExpr, cs)) {
		if (!m_numberCalc) {
			NumberValueCalc* numberCalc = new NumberValueCalc();
			m_numberCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[2], *numberCalc);
			calculator.m_interpreter.Push(m_numberCalc);
		}

		if (!m_productCalc) {
			ProductCalc* productCalc = new ProductCalc();
			m_productCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *productCalc);
			calculator.m_interpreter.Push(m_productCalc);
		}

		if (m_numberCalc->m_calculation.m_state != Calculation::CalculationState::Done ||
			m_productCalc->m_calculation.m_state != Calculation::CalculationState::Done) {
			return;
		}

		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		calculator.m_calculation.m_value = Value::Multiply(m_productCalc->m_calculation.m_value, m_numberCalc->m_calculation.m_value);

		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, divideExpr, cs)) {
		if (!m_numberCalc) {
			NumberValueCalc* numberCalc = new NumberValueCalc();
			m_numberCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[2], *numberCalc);
			calculator.m_interpreter.Push(m_numberCalc);
		}

		if (!m_productCalc) {
			ProductCalc* productCalc = new ProductCalc();
			m_productCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *productCalc);
			calculator.m_interpreter.Push(m_productCalc);
		}

		if (m_numberCalc->m_calculation.m_state != Calculation::CalculationState::Done ||
			m_productCalc->m_calculation.m_state != Calculation::CalculationState::Done) {
			return;
		}

		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		calculator.m_calculation.m_value = Value::Divide(m_productCalc->m_calculation.m_value, m_numberCalc->m_calculation.m_value);

		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, quotientExpr, cs)) {
		if (!m_numberCalc) {
			NumberValueCalc* numberCalc = new NumberValueCalc();
			m_numberCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[2], *numberCalc);
			calculator.m_interpreter.Push(m_numberCalc);
		}

		if (!m_productCalc) {
			ProductCalc* productCalc = new ProductCalc();
			m_productCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *productCalc);
			calculator.m_interpreter.Push(m_productCalc);
		}

		if (m_numberCalc->m_calculation.m_state != Calculation::CalculationState::Done ||
			m_productCalc->m_calculation.m_state != Calculation::CalculationState::Done) {
			return;
		}

		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		calculator.m_calculation.m_value = Value::Quotient(m_productCalc->m_calculation.m_value, m_numberCalc->m_calculation.m_value);

		return;
	}


	calculator.m_calculation = Calculation::GetFailed();
}

interpreter::ProductCalc::~ProductCalc()
{
}

void interpreter::ProductCalc::FreeUpResouces()
{
	if (m_numberCalc) {
		delete m_numberCalc;
	}

	if (m_productCalc) {
		delete m_productCalc;
	}

	m_numberCalc = nullptr;
	m_productCalc = nullptr;
}

void interpreter::NumberValueCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;

	const char* rawValueExpr[] = { "RawValue", 0 };
	const char* negative[] = { "-", "NumberValue", 0 };
	const char* sum[] = { "(", "Sum", ")", 0};


	if (symbolUtils::MatchChildren(&calculator.m_symbol, rawValueExpr, cs)) {
		if (!m_calc) {
			RawValueCalc* rawValueCalc = new RawValueCalc();
			m_calc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *rawValueCalc);
			calculator.m_interpreter.Push(m_calc);
		}

		calculator.m_calculation = m_calc->m_calculation;

		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, negative, cs)) {
		if (!m_calc) {
			NumberValueCalc* numValueCalc = new NumberValueCalc();
			m_calc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[1], *numValueCalc);
			calculator.m_interpreter.Push(m_calc);
		}

		if (m_calc->m_calculation.m_state != Calculation::CalculationState::Done) {
			return;
		}

		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		calculator.m_calculation.m_value = Value::Negate(m_calc->m_calculation.m_value);
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, sum, cs)) {
		if (!m_calc) {
			SumCalc* sumCalc = new SumCalc();
			m_calc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[1], *sumCalc);
			calculator.m_interpreter.Push(m_calc);
		}

		if (m_calc->m_calculation.m_state != Calculation::CalculationState::Done) {
			return;
		}

		calculator.m_calculation = m_calc->m_calculation;
		return;
	}

	calculator.m_calculation = Calculation::GetFailed();
}

interpreter::NumberValueCalc::~NumberValueCalc()
{
}

void interpreter::NumberValueCalc::FreeUpResouces()
{
	if (m_calc) {
		delete m_calc;
	}

	m_calc = nullptr;
}

void interpreter::RawValueCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;

	const char* number[] = { "Number", 0 };
	const char* str[] = { "String", 0 };
	const char* name[] = { "Name", 0 };
	const char* funcDef[] = { "FuncDef", 0 };
	const char* funcCall[] = { "FuncCall", 0 };
	const char* list[] = { "[", "]", 0 };
	const char* object[] = { "{", "}", 0 };
	const char* none[] = { "none", 0 };

	const char* prop[] = { "RawValue", ".", "Name", 0 };
	const char* propExpr[] = { "RawValue", "[", "Expression", "]", 0 };

	if (symbolUtils::MatchChildren(&calculator.m_symbol, none, cs)) {
		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		calculator.m_calculation.m_value = Value();
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, number, cs)) {
		calculator.m_calculation.m_state = Calculation::Done;
		calculator.m_calculation.m_value = Value(cs->m_childSymbols[0]->m_symbolData.m_number);
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, str, cs)) {
		calculator.m_calculation.m_state = Calculation::Done;
		calculator.m_calculation.m_value = Value(cs->m_childSymbols[0]->m_symbolData.m_string);
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, name, cs)) {
		calculator.m_calculation.m_state = Calculation::Done;
		std::string varName = cs->m_childSymbols[0]->m_symbolData.m_string;
		calculator.m_calculation.m_value = calculator.m_interpreter.GetCurrentScope().GetValue(varName);
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, list, cs)) {
		calculator.m_calculation.m_state = Calculation::Done;

		calculator.m_calculation.m_value = ListValue::Create();
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, object, cs)) {
		calculator.m_calculation.m_state = Calculation::Done;

		calculator.m_calculation.m_value = ObjectValue::Create();
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, funcDef, cs)) {
		if (!m_calc) {
			FuncDefCalc* funcDefCalc = new FuncDefCalc();
			m_calc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *funcDefCalc);
			calculator.m_interpreter.Push(m_calc);
		}
		
		calculator.m_calculation = m_calc->m_calculation;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, funcCall, cs)) {
		if (!m_calc) {
			FuncCallCalc* funcCallCalc = new FuncCallCalc();
			m_calc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *funcCallCalc);
			calculator.m_interpreter.Push(m_calc);
		}

		calculator.m_calculation = m_calc->m_calculation;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, prop, cs)) {

		if (!m_calc) {
			RawValueCalc* rawValueCalc = new RawValueCalc();
			m_calc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *rawValueCalc);
			calculator.m_interpreter.Push(m_calc);
		}

		if (m_calc->m_calculation.m_state == Calculation::CalculationState::Pending) {
			return;
		}

		if (m_calc->m_calculation.m_value.GetType() != ScriptingValueType::Object) {
			calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
			return;
		}

		std::string propName = cs->m_childSymbols[2]->m_symbolData.m_string;

		IManagedValue* managedValue = m_calc->m_calculation.m_value.GetManagedValue();

		calculator.m_calculation.m_value = m_calc->m_calculation.m_value.GetManagedValue()->GetProperty(propName);
		calculator.m_calculation.m_state = Calculation::CalculationState::Done;

		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, propExpr, cs)) {
		if (!m_expr) {
			ExpressionCalc* expr = new ExpressionCalc();
			m_expr = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[2], *expr);
			calculator.m_interpreter.Push(m_expr);
		}

		if (!m_calc) {
			RawValueCalc* rawValueCalc = new RawValueCalc();
			m_calc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *rawValueCalc);
			calculator.m_interpreter.Push(m_calc);
		}

		if (m_calc->m_calculation.m_state == Calculation::CalculationState::Pending) {
			return;
		}

		if (m_calc->m_calculation.m_value.GetType() != ScriptingValueType::Object) {
			calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
			return;
		}

		if (m_expr->m_calculation.m_state == Calculation::CalculationState::Pending) {
			return;
		}

		if (m_expr->m_calculation.m_value.GetType() != ScriptingValueType::String && 
			m_expr->m_calculation.m_value.GetType() != ScriptingValueType::Number) {
			calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
			return;
		}

		
		if (m_expr->m_calculation.m_value.GetType() == ScriptingValueType::String) {
			calculator.m_calculation.m_value =
				m_calc->m_calculation.m_value.GetManagedValue()->GetProperty(m_expr->m_calculation.m_value.GetString());
			calculator.m_calculation.m_state = Calculation::CalculationState::Done;
			return;
		}

		if (m_expr->m_calculation.m_value.GetType() == ScriptingValueType::Number) {
			ListValue* list = dynamic_cast<ListValue*>(m_calc->m_calculation.m_value.GetManagedValue());
			if (!list) {
				calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
				return;
			}

			calculator.m_calculation.m_value =
				list->GetValueAt(m_expr->m_calculation.m_value.GetNum());
			calculator.m_calculation.m_state = Calculation::CalculationState::Done;
			return;
		}
	}

	calculator.m_calculation = Calculation::GetFailed();
}

interpreter::RawValueCalc::~RawValueCalc()
{
}

void interpreter::RawValueCalc::FreeUpResouces()
{
	if (m_calc) {
		delete m_calc;
	}
	if (m_expr) {
		delete m_expr;
	}

	m_calc = nullptr;
	m_expr = nullptr;
}

void interpreter::ICalculator::FreeUpResouces()
{
}

interpreter::ICalculator::~ICalculator()
{
	FreeUpResouces();
}

void interpreter::BinaryExpressionCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;

	const char* disjunctionExpr[] = { "Disjunction", 0 };

	if (symbolUtils::MatchChildren(&calculator.m_symbol, disjunctionExpr, cs)) {
		if (!m_disjunctionCalc) {
			DisjunctionCalc* disjunctionCalc = new DisjunctionCalc();
			m_disjunctionCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *disjunctionCalc);
			calculator.m_interpreter.Push(m_disjunctionCalc);
		}

		calculator.m_calculation = m_disjunctionCalc->m_calculation;
		return;
	}

	calculator.m_calculation = Calculation::GetFailed();
}

interpreter::BinaryExpressionCalc::~BinaryExpressionCalc()
{
}

void interpreter::BinaryExpressionCalc::FreeUpResouces()
{
	if (m_disjunctionCalc) {
		delete m_disjunctionCalc;
	}

	m_disjunctionCalc = nullptr;
}

void interpreter::DisjunctionCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;

	const char* single[] = { "Conjunction", 0 };
	const char* multi[] = { "Disjunction", "|", "|", "Conjunction", 0};

	if (symbolUtils::MatchChildren(&calculator.m_symbol, single, cs)) {
		if (!m_conjunctionCalc) {
			ConjunctionCalc* conjunctionCalc = new ConjunctionCalc();
			m_conjunctionCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *conjunctionCalc);
			calculator.m_interpreter.Push(m_conjunctionCalc);
		}

		calculator.m_calculation = m_conjunctionCalc->m_calculation;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, multi, cs)) {
		if (!m_conjunctionCalc) {
			ConjunctionCalc* conjunctionCalc = new ConjunctionCalc();
			m_conjunctionCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[3], *conjunctionCalc);
			calculator.m_interpreter.Push(m_conjunctionCalc);
		}

		if (!m_disjunctionCalc) {
			DisjunctionCalc* disjunctionCalc = new DisjunctionCalc();
			m_disjunctionCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *disjunctionCalc);
			calculator.m_interpreter.Push(m_disjunctionCalc);
		}

		if (m_disjunctionCalc->m_calculation.m_state != Calculation::Done) {
			return;
		}

		if (m_conjunctionCalc->m_calculation.m_state != Calculation::Done) {
			return;
		}

		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		calculator.m_calculation.m_value = Value::Or(m_disjunctionCalc->m_calculation.m_value, m_conjunctionCalc->m_calculation.m_value);

		return;
	}

	calculator.m_calculation = Calculation::GetFailed();
}

interpreter::DisjunctionCalc::~DisjunctionCalc()
{
}

void interpreter::DisjunctionCalc::FreeUpResouces()
{
	if (m_disjunctionCalc) {
		delete m_disjunctionCalc;
	}

	if (m_conjunctionCalc) {
		delete m_conjunctionCalc;
	}

	m_disjunctionCalc = nullptr;
	m_conjunctionCalc = nullptr;
}

void interpreter::ConjunctionCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;

	const char* single[] = { "BinaryValue", 0 };
	const char* multi[] = { "Conjunction", "&", "&", "BinaryValue", 0 };

	if (symbolUtils::MatchChildren(&calculator.m_symbol, single, cs)) {
		if (!m_binaryValueCalc) {
			BinaryValueCalc* binValueCalc = new BinaryValueCalc();
			m_binaryValueCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *binValueCalc);
			calculator.m_interpreter.Push(m_binaryValueCalc);
		}

		calculator.m_calculation = m_binaryValueCalc->m_calculation;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, multi, cs)) {
		if (!m_binaryValueCalc) {
			BinaryValueCalc* binValueCalc = new BinaryValueCalc();
			m_binaryValueCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[3], *binValueCalc);
			calculator.m_interpreter.Push(m_binaryValueCalc);
		}

		if (!m_conjunctionCalc) {
			ConjunctionCalc* conjunctionCalc = new ConjunctionCalc();
			m_conjunctionCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *conjunctionCalc);
			calculator.m_interpreter.Push(m_conjunctionCalc);
		}

		if (m_binaryValueCalc->m_calculation.m_state != Calculation::Done) {
			return;
		}

		if (m_conjunctionCalc->m_calculation.m_state != Calculation::Done) {
			return;
		}

		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		calculator.m_calculation.m_value = Value::And(m_conjunctionCalc->m_calculation.m_value, m_binaryValueCalc->m_calculation.m_value);
		return;
	}

	calculator.m_calculation = Calculation::GetFailed();
}


interpreter::ConjunctionCalc::~ConjunctionCalc()
{
}

void interpreter::ConjunctionCalc::FreeUpResouces()
{
	if (m_conjunctionCalc) {
		delete m_conjunctionCalc;
	}
	if (m_binaryValueCalc) {
		delete m_binaryValueCalc;
	}

	m_conjunctionCalc = nullptr;
	m_binaryValueCalc = nullptr;
}

void interpreter::BinaryValueCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;

	const char* negation[] = { "!", "BinaryValue", 0 };
	const char* disjunction[] = { "(", "Disjunction", ")", 0 };
	const char* comparison[] = { "Comparison", 0 };

	if (symbolUtils::MatchChildren(&calculator.m_symbol, negation, cs)) {
		if (!m_binValCalc) {
			BinaryValueCalc* binValueCalc = new BinaryValueCalc();
			m_binValCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[1], *binValueCalc);
			calculator.m_interpreter.Push(m_binValCalc);
		}

		if (m_binValCalc->m_calculation.m_state != Calculation::Done) {
			return;
		}

		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		calculator.m_calculation.m_value = Value::Not(m_binValCalc->m_calculation.m_value);

		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, disjunction, cs)) {
		if (!m_disjunctionCalc) {
			DisjunctionCalc* disjunctionCalc = new DisjunctionCalc();
			m_disjunctionCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[1], *disjunctionCalc);
			calculator.m_interpreter.Push(m_disjunctionCalc);
		}

		calculator.m_calculation = m_disjunctionCalc->m_calculation;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, comparison, cs)) {
		if (!m_comparisonCalc) {
			ComparisonCalc* comparisonCalc = new ComparisonCalc();
			m_comparisonCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *comparisonCalc);
			calculator.m_interpreter.Push(m_comparisonCalc);
		}

		calculator.m_calculation = m_comparisonCalc->m_calculation;
		return;
	}

	calculator.m_calculation = Calculation::GetFailed();
}

interpreter::BinaryValueCalc::~BinaryValueCalc()
{
}

void interpreter::BinaryValueCalc::FreeUpResouces()
{
	if (m_binValCalc) {
		delete m_binValCalc;
	}

	if (m_disjunctionCalc) {
		delete m_disjunctionCalc;
	}

	if (m_comparisonCalc) {
		delete m_comparisonCalc;
	}

	m_binValCalc = nullptr;
	m_disjunctionCalc = nullptr;
	m_comparisonCalc = nullptr;
}

void interpreter::ComparisonCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;

	const char* equal[] = { "ArithmeticExpression", "=", "=", "ArithmeticExpression", 0 };
	const char* notEqual[] = { "ArithmeticExpression", "!", "=", "ArithmeticExpression", 0 };
	const char* lessOrEqual[] = { "ArithmeticExpression", "<", "=", "ArithmeticExpression", 0 };
	const char* greaterOrEqual[] = { "ArithmeticExpression", ">", "=", "ArithmeticExpression", 0 };
	const char* less[] = { "ArithmeticExpression", "<", "ArithmeticExpression", 0 };
	const char* greater[] = { "ArithmeticExpression", ">", "ArithmeticExpression", 0 };

	bool b_equal = false;
	bool b_notEqual = false;
	bool b_less = false;
	bool b_greater = false;
	bool b_lessOrEqual = false;
	bool b_greaterOrEqual = false;

	if (symbolUtils::MatchChildren(&calculator.m_symbol, equal, cs)) {
		b_equal = true;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, notEqual, cs)) {
		b_notEqual = true;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, less, cs)) {
		b_less = true;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, lessOrEqual, cs)) {
		b_lessOrEqual = true;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, greater, cs)) {
		b_greater = true;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, greaterOrEqual, cs)) {
		b_greaterOrEqual = true;
	}

	int leftIndex = 0;
	int rightIndex = 2;
	if (b_equal || b_notEqual || b_lessOrEqual || b_greaterOrEqual) {
		rightIndex = 3;
	}

	if (!m_rightExpressionCalc) {
		ArithmethicExpressionCalc* arithmeticExprCalc = new ArithmethicExpressionCalc();
		m_rightExpressionCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[rightIndex], *arithmeticExprCalc);
		calculator.m_interpreter.Push(m_rightExpressionCalc);
	}

	if (!m_leftExpressionCalc) {
		ArithmethicExpressionCalc* arithmeticExprCalc = new ArithmethicExpressionCalc();
		m_leftExpressionCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[leftIndex], *arithmeticExprCalc);
		calculator.m_interpreter.Push(m_leftExpressionCalc);
	}

	if (m_rightExpressionCalc->m_calculation.m_state != Calculation::Done) {
		return;
	}

	if (m_leftExpressionCalc->m_calculation.m_state != Calculation::Done) {
		return;
	}

	if (b_equal) {
		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		calculator.m_calculation.m_value = Value::Equal(m_leftExpressionCalc->m_calculation.m_value, m_rightExpressionCalc->m_calculation.m_value);
		return;
	}

	if (b_notEqual) {
		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		Value tmp = Value::Equal(m_leftExpressionCalc->m_calculation.m_value, m_rightExpressionCalc->m_calculation.m_value);
		calculator.m_calculation.m_value = Value::Not(tmp);
		return;
	}
	
	if (b_less) {
		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		calculator.m_calculation.m_value = Value::Less(m_leftExpressionCalc->m_calculation.m_value, m_rightExpressionCalc->m_calculation.m_value);
		return;
	}

	if (b_lessOrEqual) {
		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		calculator.m_calculation.m_value = Value::LessOrEqual(m_leftExpressionCalc->m_calculation.m_value, m_rightExpressionCalc->m_calculation.m_value);
		return;
	}

	if (b_greater) {
		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		calculator.m_calculation.m_value = Value::Greater(m_leftExpressionCalc->m_calculation.m_value, m_rightExpressionCalc->m_calculation.m_value);
		return;
	}

	if (b_greaterOrEqual) {
		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		calculator.m_calculation.m_value = Value::GreaterOrEqual(m_leftExpressionCalc->m_calculation.m_value, m_rightExpressionCalc->m_calculation.m_value);
		return;
	}

	calculator.m_calculation = Calculation::GetFailed();
}

interpreter::ComparisonCalc::~ComparisonCalc()
{
}

void interpreter::ComparisonCalc::FreeUpResouces()
{
	if (m_leftExpressionCalc) {
		delete m_leftExpressionCalc;
	}

	if (m_rightExpressionCalc) {
		delete m_rightExpressionCalc;
	}

	m_leftExpressionCalc = nullptr;
	m_rightExpressionCalc = nullptr;
}

void interpreter::IfStatementCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs = dynamic_cast<const scripting::CompositeSymbol*>(&calculator.m_symbol);

	if (!m_expressionCalc) {
		ExpressionCalc* calc = new ExpressionCalc();
		m_expressionCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[2], *calc);
		calculator.m_interpreter.Push(m_expressionCalc);
	}

	if (m_expressionCalc->m_calculation.m_state == Calculation::CalculationState::Pending) {
		return;
	}

	bool clause = m_expressionCalc->m_calculation.m_value.IsTrue();

	if (!clause) {
		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		return;
	}

	if (!m_instructionsBlockCalc) {
		InstructionsBlockCalc* calc = new InstructionsBlockCalc();
		m_instructionsBlockCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[4], *calc);
		calculator.m_interpreter.Push(m_instructionsBlockCalc);
	}

	calculator.m_calculation.m_state = m_instructionsBlockCalc->m_calculation.m_state;
}

void interpreter::IfStatementCalc::FreeUpResouces()
{
	if (m_expressionCalc) {
		delete m_expressionCalc;
	}

	if (m_instructionsBlockCalc) {
		delete m_instructionsBlockCalc;
	}

	m_expressionCalc = nullptr;
	m_instructionsBlockCalc = nullptr;
}

interpreter::IfStatementCalc::~IfStatementCalc()
{
}

void interpreter::InstructionsBlockCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs = dynamic_cast<const scripting::CompositeSymbol*>(&calculator.m_symbol);

	if (!m_instructionsCalc) {
		InstructionsCalc* instructionsCalc = new InstructionsCalc();
		m_instructionsCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[1], *instructionsCalc);
		calculator.m_interpreter.Push(m_instructionsCalc);

		calculator.m_interpreter.PushScope();
	}

	calculator.m_calculation = m_instructionsCalc->m_calculation;
	if (calculator.m_calculation.m_state == Calculation::Done) {
		calculator.m_interpreter.PopScope();
	}
}

void interpreter::InstructionsBlockCalc::FreeUpResouces()
{
	if (m_instructionsCalc) {
		delete m_instructionsCalc;
	}

	m_instructionsCalc = nullptr;
}

interpreter::InstructionsBlockCalc::~InstructionsBlockCalc()
{
}

void interpreter::WhileStatementCalc::Calculate(Calculator& calculator)
{
	if (m_breakInstruction) {
		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		calculator.m_interpreter.m_scope = m_currentInterpreterScope;
		return;
	}

	if (m_continueInstruction) {
		calculator.m_interpreter.m_scope = m_currentInterpreterScope;
		FreeUpResouces();
		return;
	}

	const scripting::CompositeSymbol* cs = dynamic_cast<const scripting::CompositeSymbol*>(&calculator.m_symbol);

	if (!m_expressionCalc) {
		ExpressionCalc* calc = new ExpressionCalc();
		m_expressionCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[2], *calc);
		calculator.m_interpreter.Push(m_expressionCalc);
	}

	if (m_expressionCalc->m_calculation.m_state == Calculation::CalculationState::Pending) {
		return;
	}

	bool clause = m_expressionCalc->m_calculation.m_value.IsTrue();

	if (!clause) {
		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		return;
	}

	m_currentInterpreterScope = calculator.m_interpreter.m_scope;

	if (!m_instructionsBlockCalc) {
		InstructionsBlockCalc* calc = new InstructionsBlockCalc();
		m_instructionsBlockCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[4], *calc);
		calculator.m_interpreter.Push(m_instructionsBlockCalc);
	}

	if (m_instructionsBlockCalc->m_calculation.m_state == Calculation::CalculationState::Pending) {
		return;
	}

	if (m_instructionsBlockCalc->m_calculation.m_state == Calculation::CalculationState::Failed) {
		calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
		return;
	}

	if (m_expressionCalc) {
		delete m_expressionCalc;
	}
	if (m_instructionsBlockCalc) {
		delete m_instructionsBlockCalc;
	}

	m_expressionCalc = nullptr;
	m_instructionsBlockCalc = nullptr;
}

void interpreter::WhileStatementCalc::FreeUpResouces()
{
	if (m_expressionCalc) {
		delete m_expressionCalc;
	}
	if (m_instructionsBlockCalc) {
		delete m_instructionsBlockCalc;
	}

	m_expressionCalc = nullptr;
	m_instructionsBlockCalc = nullptr;
}

interpreter::WhileStatementCalc::~WhileStatementCalc()
{
}

void interpreter::LValueCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;

	const char* justName[] = { "Name", 0 };
	const char* prop[] = { "RawValue", ".", "Name", 0 };
	const char* propExpr[] = { "RawValue", "[", "Expression", "]", 0 };

	if (symbolUtils::MatchChildren(&calculator.m_symbol, justName, cs)) {
		m_prop = Value(cs->m_childSymbols[0]->m_symbolData.m_string);
		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, prop, cs)) {

		if (!m_calc) {
			RawValueCalc* rawValueCalc = new RawValueCalc();
			m_calc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *rawValueCalc);
			calculator.m_interpreter.Push(m_calc);
		}

		if (m_calc->m_calculation.m_state == Calculation::CalculationState::Pending) {
			return;
		}

		if (m_calc->m_calculation.m_value.GetType() != ScriptingValueType::Object) {
			calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
			return;
		}

		std::string propName = cs->m_childSymbols[2]->m_symbolData.m_string;

		m_prop = Value(propName);
		m_outerObject = m_calc->m_calculation.m_value;
		calculator.m_calculation.m_state = Calculation::CalculationState::Done;

		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, propExpr, cs)) {
		if (!m_expr) {
			ExpressionCalc* expr = new ExpressionCalc();
			m_expr = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[2], *expr);
			calculator.m_interpreter.Push(m_expr);
		}

		if (!m_calc) {
			RawValueCalc* rawValueCalc = new RawValueCalc();
			m_calc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *rawValueCalc);
			calculator.m_interpreter.Push(m_calc);
		}

		if (m_calc->m_calculation.m_state == Calculation::CalculationState::Pending) {
			return;
		}

		if (m_calc->m_calculation.m_value.GetType() != ScriptingValueType::Object) {
			calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
			return;
		}

		if (m_expr->m_calculation.m_state == Calculation::CalculationState::Pending) {
			return;
		}

		if (m_expr->m_calculation.m_value.GetType() != ScriptingValueType::String &&
			m_expr->m_calculation.m_value.GetType() != ScriptingValueType::Number) {
			calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
			return;
		}


		if (m_expr->m_calculation.m_value.GetType() == ScriptingValueType::String) {
			m_prop = m_expr->m_calculation.m_value;
			m_outerObject = m_calc->m_calculation.m_value;

			calculator.m_calculation.m_state = Calculation::CalculationState::Done;
			return;
		}

		if (m_expr->m_calculation.m_value.GetType() == ScriptingValueType::Number) {
			m_prop = m_expr->m_calculation.m_value;
			m_outerObject = m_calc->m_calculation.m_value;

			calculator.m_calculation.m_state = Calculation::CalculationState::Done;
			return;
		}
	}

	calculator.m_calculation.m_state = Calculation::Failed;
}

void interpreter::LValueCalc::FreeUpResouces()
{
	if (m_calc) {
		delete m_calc;
	}
	if (m_expr) {
		delete m_expr;
	}

	m_calc = nullptr;
	m_expr = nullptr;
}

interpreter::LValueCalc::~LValueCalc()
{
}

void interpreter::VarDefCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;

	const char* varDef [] = { "let", "Name", "=", "Expression", ";", 0 };

	if (!symbolUtils::MatchChildren(&calculator.m_symbol, varDef, cs)) {
		calculator.m_calculation = Calculation::GetFailed();
		return;
	}

	if (!m_calc) {
		ExpressionCalc* expressionCalc = new ExpressionCalc();
		m_calc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[3], *expressionCalc);
		calculator.m_interpreter.Push(m_calc);
	}

	if (m_calc->m_calculation.m_state == Calculation::CalculationState::Pending) {
		return;
	}

	std::string name = cs->m_childSymbols[1]->m_symbolData.m_string;

	calculator.m_interpreter.GetCurrentScope().BindValue(name, m_calc->m_calculation.m_value);
	calculator.m_calculation.m_state = Calculation::CalculationState::Done;
}

void interpreter::VarDefCalc::FreeUpResouces()
{
	if (m_calc) {
		delete m_calc;
	}

	m_calc = nullptr;
}

interpreter::VarDefCalc::~VarDefCalc()
{
}

void interpreter::ArgumentsCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;

	const char* expr[] = { "Expression", 0 };
	const char* multiArgs[] = { "Arguments", ",", "Expression", 0 };

	if (symbolUtils::MatchChildren(&calculator.m_symbol, expr, cs)) {
		if (!m_exprCalc) {
			ExpressionCalc* exprCalc = new ExpressionCalc();
			m_exprCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *exprCalc);
			calculator.m_interpreter.Push(m_exprCalc);
		}

		if (m_exprCalc->m_calculation.m_state == Calculation::CalculationState::Done) {
			calculator.m_calculation.m_state = Calculation::CalculationState::Done;
			m_args.push_back(m_exprCalc->m_calculation.m_value);
		}
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, multiArgs, cs)) {
		if (!m_exprCalc) {
			ExpressionCalc* exprCalc = new ExpressionCalc();
			m_exprCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[2], *exprCalc);
			calculator.m_interpreter.Push(m_exprCalc);
		}
		
		if (!m_argumentsCalc) {
			ArgumentsCalc* argsCalc = new ArgumentsCalc();
			m_argumentsCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *argsCalc);
			calculator.m_interpreter.Push(m_argumentsCalc);
		}

		if (m_argumentsCalc->m_calculation.m_state != Calculation::CalculationState::Done) {
			return;
		}
		if (m_exprCalc->m_calculation.m_state != Calculation::CalculationState::Done) {
			return;
		}

		m_args = ((ArgumentsCalc&) m_argumentsCalc->m_calculator).m_args;
		m_args.push_back(m_exprCalc->m_calculation.m_value);

		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		return;
	}
	calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
}

void interpreter::ArgumentsCalc::FreeUpResouces()
{
	if (m_argumentsCalc) {
		delete m_argumentsCalc;
	}
	if (m_exprCalc) {
		delete m_exprCalc;
	}

	m_argumentsCalc = nullptr;
	m_exprCalc = nullptr;

}

interpreter::ArgumentsCalc::~ArgumentsCalc()
{
}

void interpreter::ArgumentListCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;

	const char* empty[] = { "(", ")", 0 };
	const char* multiArgs[] = { "(", "Arguments", ")", 0 };

	if (symbolUtils::MatchChildren(&calculator.m_symbol, empty, cs)) {
		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, multiArgs, cs)) {
		if (!m_argsCalc) {
			ArgumentsCalc* argsCalc = new ArgumentsCalc();
			m_argsCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[1], *argsCalc);
			calculator.m_interpreter.Push(m_argsCalc);
		}

		if (m_argsCalc->m_calculation.m_state == Calculation::CalculationState::Done) {
			ArgumentsCalc& argsCalc = (ArgumentsCalc&)m_argsCalc->m_calculator;
			m_args = argsCalc.m_args;
			calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		}

		return;
	}

	calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
}

void interpreter::ArgumentListCalc::FreeUpResouces()
{
	if (m_argsCalc) {
		delete m_argsCalc;
	}
	m_argsCalc = nullptr;
}

interpreter::ArgumentListCalc::~ArgumentListCalc()
{
}

void interpreter::FuncCallCalc::Calculate(Calculator& calculator)
{
	if (m_returnInstruction) {
		calculator.m_interpreter.m_scope = m_curInterpreterScope;
		calculator.m_calculation.m_state = Calculation::Done;
		return;
	}

	const scripting::CompositeSymbol* cs;

	const char* funcCall[] = { "RawValue", "ArgumentList", 0 };

	if (!symbolUtils::MatchChildren(&calculator.m_symbol, funcCall, cs)) {
		calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
		return;
	}

	if (!m_argsList) {
		ArgumentListCalc* argsList = new ArgumentListCalc();
		m_argsList = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[1], *argsList);
		calculator.m_interpreter.Push(m_argsList);
	}
	
	if (!m_funcCalc) {
		RawValueCalc* funcCalc = new RawValueCalc();
		m_funcCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *funcCalc);
		calculator.m_interpreter.Push(m_funcCalc);
	}

	if (m_funcCalc->m_calculation.m_state != Calculation::CalculationState::Done) {
		return;
	}

	if (m_argsList->m_calculation.m_state != Calculation::CalculationState::Done) {
		return;
	}

	if (m_funcCalc->m_calculation.m_value.GetType() != ScriptingValueType::Object) {
		calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
		return;
	}

	IFunc* func = dynamic_cast<IFunc*>(m_funcCalc->m_calculation.m_value.GetManagedValue());
	Func* scriptingFunc = dynamic_cast<Func*>(func);

	if (!func) {
		calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
		return;
	}

	ArgumentListCalc& argsList = dynamic_cast<ArgumentListCalc&>(m_argsList->m_calculator);
	if (func->m_paramNames.size() < argsList.m_args.size()) {
		calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
		return;
	}

	if (m_curInterpreterScope.GetType() == ScriptingValueType::None) {
		m_curInterpreterScope = calculator.m_interpreter.m_scope;
		Value funcScope = func->GetScopeTemplate();

		Scope* scope = static_cast<Scope*>(funcScope.GetManagedValue());
		for (int i = 0; i < argsList.m_args.size(); ++i) {
			scope->BindValue(func->m_paramNames[i], argsList.m_args[i]);
		}

		calculator.m_interpreter.m_scope = funcScope;
	}

	Scope* scope = dynamic_cast<Scope*>(calculator.m_interpreter.m_scope.GetManagedValue());

	if (!scriptingFunc) {
		FuncResult res = func->Execute(*scope);

		if (res.m_state != FuncResult::FuncExecutionState::Pending) {
			calculator.m_interpreter.m_scope = m_curInterpreterScope;
		}

		if (res.m_state == FuncResult::FuncExecutionState::Failed) {
			calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
			calculator.m_calculation.m_value = res.m_returnValue;
			return;
		}

		if (res.m_state == FuncResult::FuncExecutionState::Finished) {
			calculator.m_calculation.m_state = Calculation::CalculationState::Done;
			calculator.m_calculation.m_value = res.m_returnValue;
			return;
		}

		return;
	}
	else {
		if (!m_blockCalc) {
			InstructionsBlockCalc* blockCalc = new InstructionsBlockCalc();
			m_blockCalc = new Calculator(calculator.m_interpreter, scriptingFunc->m_body, *blockCalc);
			calculator.m_interpreter.Push(m_blockCalc);
		}

		calculator.m_calculation = m_blockCalc->m_calculation;

		if (calculator.m_calculation.m_state != Calculation::CalculationState::Pending) {
			calculator.m_interpreter.m_scope = m_curInterpreterScope;
		}
		return;
	}

	calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
}

void interpreter::FuncCallCalc::FreeUpResouces()
{
	if (m_argsList) {
		delete m_argsList;
	}
	if (m_funcCalc) {
		delete m_funcCalc;
	}
	if (m_blockCalc) {
		delete m_blockCalc;
	}

	m_argsList = nullptr;
	m_funcCalc = nullptr;
	m_blockCalc = nullptr;
}

interpreter::FuncCallCalc::~FuncCallCalc()
{
}

void interpreter::ParametersCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;

	const char* name[] = { "Name", 0 };
	const char* multiParams[] = { "Parameters", ",", "Name", 0 };

	if (symbolUtils::MatchChildren(&calculator.m_symbol, name, cs)) {
		m_parameters.push_back(cs->m_childSymbols[0]->m_symbolData.m_string);
		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, multiParams, cs)) {
		if (!m_parametersCalc) {
			ParametersCalc* parametersCalc = new ParametersCalc();
			m_parametersCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[0], *parametersCalc);
			calculator.m_interpreter.Push(m_parametersCalc);
		}

		if (m_parametersCalc->m_calculation.m_state == Calculation::CalculationState::Pending) {
			return;
		}

		ParametersCalc& parametersCalc = dynamic_cast<ParametersCalc&>(m_parametersCalc->m_calculator);
		m_parameters = parametersCalc.m_parameters;
		m_parameters.push_back(cs->m_childSymbols[2]->m_symbolData.m_string);


		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		return;
	}
	calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
}

void interpreter::ParametersCalc::FreeUpResouces()
{
	if (m_parametersCalc) {
		delete m_parametersCalc;
	}

	m_parametersCalc = nullptr;
}

interpreter::ParametersCalc::~ParametersCalc()
{
}

void interpreter::ParameterListCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;

	const char* empty[] = { "(", ")", 0 };
	const char* multiParams[] = { "(", "Parameters", ")", 0 };

	if (symbolUtils::MatchChildren(&calculator.m_symbol, empty, cs)) {
		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		return;
	}

	if (symbolUtils::MatchChildren(&calculator.m_symbol, multiParams, cs)) {
		if (!m_paramsCalc) {
			ParametersCalc* paramsCalc = new ParametersCalc();
			m_paramsCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[1], *paramsCalc);
			calculator.m_interpreter.Push(m_paramsCalc);
		}

		if (m_paramsCalc->m_calculation.m_state != Calculation::CalculationState::Done) {
			return;
		}

		ParametersCalc& argsCalc = dynamic_cast<ParametersCalc&>(m_paramsCalc->m_calculator);
		m_parameters = argsCalc.m_parameters;
		calculator.m_calculation.m_state = Calculation::CalculationState::Done;
		return;
	}

	calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
}

void interpreter::ParameterListCalc::FreeUpResouces()
{
	if (m_paramsCalc) {
		delete m_paramsCalc;
	}

	m_paramsCalc = nullptr;
}

interpreter::ParameterListCalc::~ParameterListCalc()
{
}

void interpreter::FuncDefCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;
	const char* funcDef[] = { "func", "ParameterList", "InstructionsBlock", 0 };

	if (!symbolUtils::MatchChildren(&calculator.m_symbol, funcDef, cs)) {
		calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
		return;
	}

	if (!m_parameterListCalc) {
		ParameterListCalc* parameterListCalc = new ParameterListCalc();
		m_parameterListCalc = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[1], *parameterListCalc);
		calculator.m_interpreter.Push(m_parameterListCalc);
	}

	if (m_parameterListCalc->m_calculation.m_state == Calculation::CalculationState::Pending) {
		return;
	}

	ParameterListCalc& parameterListCalc = dynamic_cast<ParameterListCalc&>(m_parameterListCalc->m_calculator);

	Func* f = new Func(*cs->m_childSymbols[2]);
	Value fWrapped(*f);
	f->InitFuncDefScope(calculator.m_interpreter.m_scope);
	f->m_paramNames = parameterListCalc.m_parameters;

	calculator.m_calculation.m_state = Calculation::CalculationState::Done;
	calculator.m_calculation.m_value = fWrapped;
}

void interpreter::FuncDefCalc::FreeUpResouces()
{
	if (m_parameterListCalc) {
		delete m_parameterListCalc;
	}

	m_parameterListCalc = nullptr;
}

interpreter::FuncDefCalc::~FuncDefCalc()
{
}

void interpreter::TryCatchCalc::Calculate(Calculator& calculator)
{
	const scripting::CompositeSymbol* cs;
	const char* tryCatch[] = { "try", "InstructionsBlock", "catch", "(", "Name", ")", "InstructionsBlock", 0 };

	if (!symbolUtils::MatchChildren(&calculator.m_symbol, tryCatch, cs)) {
		calculator.m_calculation.m_state = Calculation::CalculationState::Failed;
		return;
	}

	if (m_curInterpreterScope.IsNone()) {
		m_curInterpreterScope = calculator.m_interpreter.m_scope;
	}

	if (!m_exception) {
		if (!m_block) {
			InstructionsBlockCalc* calc = new InstructionsBlockCalc();
			m_block = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[1], *calc);
			calculator.m_interpreter.Push(m_block);
		}

		calculator.m_calculation = m_block->m_calculation;
		return;
	}

	if (m_exceptionScope.IsNone()) {
		m_exceptionScope = Scope::Create();
		Scope* scope = static_cast<Scope*>(m_exceptionScope.GetManagedValue());
		scope->BindValue(cs->m_childSymbols[4]->m_symbolData.m_string, m_exceptionValue);
		scope->SetParentScope(m_curInterpreterScope);

		calculator.m_interpreter.m_scope = m_exceptionScope;
	}

	if (!m_catchBlock) {
		InstructionsBlockCalc* calc = new InstructionsBlockCalc();
		m_catchBlock = new Calculator(calculator.m_interpreter, *cs->m_childSymbols[6], *calc);
		calculator.m_interpreter.Push(m_catchBlock);
	}

	calculator.m_calculation = m_catchBlock->m_calculation;
	if (calculator.m_calculation.m_state != Calculation::CalculationState::Pending) {
		calculator.m_interpreter.m_scope = m_curInterpreterScope;
	}
}

void interpreter::TryCatchCalc::FreeUpResouces()
{
	if (m_block) {
		delete m_block;
	}
	if (m_catchBlock) {
		delete m_catchBlock;
	}

	m_block = nullptr;
	m_catchBlock = nullptr;
}

interpreter::TryCatchCalc::~TryCatchCalc()
{
}
