#pragma once

#include "scriptingValue.h"

#include <vector>
#include <string>

namespace interpreter
{
	struct Scope;

	struct FuncResult
	{
		enum FuncExecutionState
		{
			Pending,
			Finished,
			Failed
		};

		FuncExecutionState m_state;
		ValueWrapper m_returnValue;
	};

	struct IFunc : public IManagedValue
	{
		std::vector<std::string> m_paramNames;
		
		virtual FuncResult Execute(Scope& scope) = 0;
		virtual ValueWrapper GetScopeTemplate();
		ValueWrapper GetArgsTemplateScope();

		void SetProperty(std::string name, ValueWrapper value) override;
		ValueWrapper GetProperty(std::string name) const override;
	};
}