#include "nativeFunc.h"

#include "IFunc.h"
#include "scope.h"

#include <string>
#include <sstream>

struct NativeFunc : public interpreter::IFunc
{
	std::function<interpreter::Value(std::vector<interpreter::Value>)> m_func;
	NativeFunc(int paramsCount, const std::function<interpreter::Value(std::vector<interpreter::Value>)>& func)
	{
		m_func = func;
		for (int i = 0; i < paramsCount; ++i) {
			std::stringstream ss;
			ss << "param" << i;
			m_paramNames.push_back(ss.str());
		}
	}

	interpreter::FuncResult Execute(interpreter::Scope& scope) override
	{
		using namespace interpreter;
		std::vector<Value> args;

		for (int i = 0; i < m_paramNames.size(); ++i) {
			Value tmp = scope.GetProperty(m_paramNames[i]);
			args.push_back(tmp);
		}

		FuncResult res;
		res.m_state = FuncResult::FuncExecutionState::Finished;
		res.m_returnValue = m_func(args);
		return res;
	}
};

interpreter::Value interpreter::CreateNativeFunc(int paramsCount, std::function<Value(std::vector<Value>)> func)
{
	NativeFunc* nativeFunc = new NativeFunc(paramsCount, func);
	return interpreter::Value(*nativeFunc);
}
