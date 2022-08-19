#pragma once

#include "value.h"

namespace interpreter::utils
{
	interpreter::Value GetEmptyObject();
	void RunCallback(const Value& func, const Value& args);
}