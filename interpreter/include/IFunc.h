#pragma once

#include "scriptingValue.h"

#include <vector>
#include <string>

namespace interpreter
{
	struct Scope;

	enum FuncExecutionState
	{
		Pending,
		Finished,
		Failed
	};

	struct FuncResult
	{
		FuncExecutionState m_state;
		ValueWrapper m_returnValue;
	};

	struct IFunc
	{
		std::vector<std::string> m_paramNames;
		ValueWrapper m_result;

		virtual FuncResult Execute(Scope& scope) = 0;
	};
}