#include "nativeFunc.h"

#include "IFunc.h"
#include "scope.h"
#include "value.h"
#include "garbageCollector.h"

#include <string>
#include <sstream>

struct NativeFunc : public interpreter::IFunc
{
	interpreter::Value m_obj;
	std::function<interpreter::Value(std::vector<interpreter::Value>)> m_func;

	NativeFunc(interpreter::Value object, int paramsCount, const std::function<interpreter::Value(std::vector<interpreter::Value>)>& func)
	{
		m_func = func;
		for (int i = 0; i < paramsCount; ++i) {
			std::stringstream ss;
			ss << "param" << i;
			m_paramNames.push_back(ss.str());
		}

		volatile interpreter::GarbageCollector::GCInstructionsBatch batch;
		m_obj = object;
		m_obj.SetImplicitRef(this);
	}

	interpreter::Value GetScopeTemplate() override
	{
		using namespace interpreter;
		Value argsScope = GetArgsTemplateScope();

		Scope* tmp = new Scope();
		Value objScope = Value(*tmp);
		tmp->BindValue("self", m_obj);

		tmp = static_cast<Scope*>(argsScope.GetManagedValue());
		tmp->SetParentScope(objScope);

		return argsScope;
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
	NativeFunc* nativeFunc = new NativeFunc(Value(), paramsCount, func);
	return interpreter::Value(*nativeFunc);
}

interpreter::Value CreateNativeMethod(interpreter::IManagedValue& object, int paramsCount, std::function<interpreter::Value(std::vector<interpreter::Value>)> func)
{
	interpreter::Value obj(object);

	NativeFunc* nativeFunc = new NativeFunc(obj, paramsCount, func);
	return interpreter::Value(*nativeFunc);
}