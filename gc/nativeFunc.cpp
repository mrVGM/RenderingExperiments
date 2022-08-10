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
	std::function<interpreter::Value(interpreter::Value)> m_func;

	interpreter::Value GetScopeTemplate() override
	{
		using namespace interpreter;
		Value argsScope = GetArgsTemplateScope();

		Value objScope = Scope::Create();
		Scope* tmp = static_cast<Scope*>(objScope.GetManagedValue());

		tmp->BindValue("self", m_obj);

		tmp = static_cast<Scope*>(argsScope.GetManagedValue());
		tmp->SetParentScope(objScope);

		return argsScope;
	}

	interpreter::FuncResult Execute(interpreter::Scope& scope) override
	{
		using namespace interpreter;

		FuncResult res;
		res.m_state = FuncResult::FuncExecutionState::Finished;
		res.m_returnValue = m_func(Value(scope));
		return res;
	}

	static interpreter::Value Create(interpreter::Value object, int paramsCount, const std::function<interpreter::Value(interpreter::Value)>& func)
	{
		NativeFunc* nativeFunc = new NativeFunc();
		interpreter::Value res(*nativeFunc);

		nativeFunc->m_func = func;

		for (int i = 0; i < paramsCount; ++i) {
			std::stringstream ss;
			ss << "param" << i;
			nativeFunc->m_paramNames.push_back(ss.str());
		}

		nativeFunc->m_obj.SetImplicitRef(*nativeFunc);
		nativeFunc->m_obj = object;

		return res;
	}

protected:
	NativeFunc()
	{
	}
};

interpreter::Value interpreter::CreateNativeFunc(int paramsCount, std::function<Value(Value)> func)
{
	return NativeFunc::Create(Value(), paramsCount, func);
}

interpreter::Value interpreter::CreateNativeMethod(IManagedValue& object, int paramsCount, std::function<Value(Value)> func)
{
	return NativeFunc::Create(Value(object), paramsCount, func);
}
