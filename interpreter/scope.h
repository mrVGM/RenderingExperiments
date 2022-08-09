#pragma once

#include <string>
#include <map>

#include "value.h"

namespace interpreter
{
	class Value;

	struct Scope : public IManagedValue
	{
		Value m_parent;

		std::map<std::string, Value> m_namedValues;

		void BindValue(std::string name, const Value& value);
		Value GetValue(std::string name);

		void SetProperty(std::string name, Value value) override;
		Value GetProperty(std::string name) const override;

		void SetParentScope(Value parentScope);

		Scope();
	};
}