#pragma once

#include "value.h"

namespace interpreter::utils
{
	interpreter::Value GetEmptyObject();
	void RunFunc(const Value& func);
}