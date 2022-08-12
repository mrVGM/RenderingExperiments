#pragma once

#include "symbol.h"
#include "value.h"

#include <string>

namespace interpreter
{
	struct Interpreter;

	struct Calculation
	{
		enum CalculationState
		{
			Pending,
			Done,
			Failed
		};

		CalculationState m_state = Calculation::Pending;
		Value m_value;

		static Calculation GetFailed();
	};

	struct ICalculator;
	struct Calculator
	{
		Interpreter& m_interpreter;
		const scripting::ISymbol& m_symbol;
		ICalculator& m_calculator;

		Calculation m_calculation;

		Calculator(Interpreter& interpreter, const scripting::ISymbol& symbol, ICalculator& m_calculator);
		~Calculator();

		void Calculate();
		void FreeUpResources();
	};

	struct ICalculator
	{
		friend struct Calculator;
		virtual void Calculate(Calculator& calculator) = 0;
	protected:
		virtual void FreeUpResouces();
		virtual ~ICalculator();
	};


	struct InitialCalc : public ICalculator
	{
		Calculator* m_instructionsCalc = nullptr;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~InitialCalc() override;
	};

	struct InstructionsCalc : public ICalculator
	{
		Calculator* m_manyInstructions = nullptr;
		Calculator* m_singleInstruction = nullptr;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~InstructionsCalc();
	};

	struct SingleInstructionCalc : public ICalculator
	{
		Calculator* m_calc = nullptr;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~SingleInstructionCalc();
	};

	struct AssignmentCalc : public ICalculator
	{
		Calculator* m_lValueCalc = nullptr;
		Calculator* m_expressionCalc = nullptr;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~AssignmentCalc() override;
	};

	struct RawValueCalc : public ICalculator
	{
		Calculator* m_calc = nullptr;
		Calculator* m_expr = nullptr;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~RawValueCalc() override;
	};

	struct ExpressionCalc : public ICalculator
	{
		Calculator* m_calc = nullptr;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~ExpressionCalc() override;
	};

	struct ArithmethicExpressionCalc : public ICalculator
	{
		Calculator* m_sumCalc = nullptr;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~ArithmethicExpressionCalc() override;
	};

	struct SumCalc : public ICalculator
	{
		Calculator* m_sumCalc = nullptr;
		Calculator* m_productCalc = nullptr;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~SumCalc() override;
	};

	struct ProductCalc : public ICalculator
	{
		Calculator* m_productCalc = nullptr;
		Calculator* m_numberCalc = nullptr;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~ProductCalc() override;
	};

	struct NumberValueCalc : public ICalculator
	{
		Calculator* m_calc = nullptr;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~NumberValueCalc() override;
	};

	struct BinaryExpressionCalc : public ICalculator
	{
		Calculator* m_disjunctionCalc = nullptr;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~BinaryExpressionCalc() override;
	};

	struct DisjunctionCalc : public ICalculator
	{
		Calculator* m_disjunctionCalc = nullptr;
		Calculator* m_conjunctionCalc = nullptr;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~DisjunctionCalc() override;
	};

	struct ConjunctionCalc : public ICalculator
	{
		Calculator* m_conjunctionCalc = nullptr;
		Calculator* m_binaryValueCalc = nullptr;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~ConjunctionCalc() override;
	};

	struct BinaryValueCalc : public ICalculator
	{
		Calculator* m_binValCalc = nullptr;
		Calculator* m_disjunctionCalc = nullptr;
		Calculator* m_comparisonCalc = nullptr;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~BinaryValueCalc() override;
	};

	struct ComparisonCalc : public ICalculator
	{
		Calculator* m_leftExpressionCalc = nullptr;
		Calculator* m_rightExpressionCalc = nullptr;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~ComparisonCalc() override;
	};

	struct IfStatementCalc : public ICalculator
	{
		Calculator* m_instructionsBlockCalc = nullptr;
		Calculator* m_expressionCalc = nullptr;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~IfStatementCalc() override;
	};

	struct InstructionsBlockCalc : public ICalculator
	{
		Calculator* m_instructionsCalc = nullptr;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~InstructionsBlockCalc() override;
	};

	struct WhileStatementCalc : public ICalculator
	{
		Calculator* m_instructionsBlockCalc = nullptr;
		Calculator* m_expressionCalc = nullptr;

		Value m_currentInterpreterScope;

		bool m_breakInstruction = false;
		bool m_continueInstruction = false;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~WhileStatementCalc() override;
	};

	struct LValueCalc : public ICalculator
	{
		Value m_prop;
		Value m_outerObject;

		Calculator* m_calc = nullptr;
		Calculator* m_expr = nullptr;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~LValueCalc() override;
	};

	struct VarDefCalc : public ICalculator
	{
		Calculator* m_calc = nullptr;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~VarDefCalc() override;
	};

	struct ArgumentsCalc : public ICalculator
	{
		Calculator* m_exprCalc = nullptr;
		Calculator* m_argumentsCalc = nullptr;

		std::vector<Value> m_args;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~ArgumentsCalc() override;
	};

	struct ArgumentListCalc : public ICalculator
	{
		Calculator* m_argsCalc = nullptr;

		std::vector<Value> m_args;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~ArgumentListCalc() override;
	};

	struct FuncCallCalc : public ICalculator
	{
		Calculator* m_funcCalc = nullptr;
		Calculator* m_argsList = nullptr;
		Calculator* m_blockCalc = nullptr;

		Value m_curInterpreterScope;
		bool m_returnInstruction = false;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~FuncCallCalc() override;
	};

	struct FuncDefCalc : public ICalculator
	{
		Calculator* m_parameterListCalc = nullptr;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~FuncDefCalc() override;
	};

	struct ParametersCalc : public ICalculator
	{
		Calculator* m_parametersCalc = nullptr;

		std::vector<std::string> m_parameters;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~ParametersCalc() override;
	};

	struct ParameterListCalc : public ICalculator
	{
		Calculator* m_paramsCalc = nullptr;

		std::vector<std::string> m_parameters;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~ParameterListCalc() override;
	};

	struct TryCatchCalc : public ICalculator
	{
		Calculator* m_block = nullptr;
		Calculator* m_catchBlock = nullptr;

		Value m_curInterpreterScope;
		Value m_exceptionScope;

		bool m_exception = false;
		Value m_exceptionValue;

		void Calculate(Calculator& calculator) override;
		void FreeUpResouces() override;
		~TryCatchCalc() override;
	};
}