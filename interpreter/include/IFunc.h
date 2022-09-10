#pragma once

#include "value.h"

#include <list>
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

		FuncExecutionState m_state = FuncExecutionState::Pending;
		Value m_returnValue;
	};

	struct IFunc : public IManagedValue
	{
		std::list<std::string> m_paramNames;
		
		virtual FuncResult Execute(Scope& scope) = 0;
		virtual Value GetScopeTemplate();
		Value GetArgsTemplateScope();

		void SetProperty(std::string name, Value value) override;
		Value GetProperty(std::string name) const override;

	protected:
		IFunc();
	};
}