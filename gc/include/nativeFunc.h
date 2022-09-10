#pragma once

#include "value.h"

#include <functional>

#include <string>

namespace interpreter
{
	Value CreateNativeFunc(int paramsCount, std::function<Value(Value)> func);
	Value CreateNativeMethod(IManagedValue& object, int paramsCount, std::function<Value(Value)> func);
}