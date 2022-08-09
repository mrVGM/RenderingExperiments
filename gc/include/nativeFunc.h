#pragma once

#include "value.h"

#include <vector>
#include <functional>

namespace interpreter
{
	Value CreateNativeFunc(int paramsCount, std::function<Value(std::vector<Value>)> func);
	Value CreateNativeMethod(IManagedValue& object, int paramsCount, std::function<Value(std::vector<Value>)> func);
}