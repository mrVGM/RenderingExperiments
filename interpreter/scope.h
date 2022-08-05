#pragma once

#include <string>
#include <map>

#include "scriptingValue.h"

namespace interpreter
{
	class ValueWrapper;

	struct Scope : public IManagedValue
	{
		ValueWrapper m_parent;

		std::map<std::string, ValueWrapper> m_namedValues;

		void BindValue(std::string name, const ValueWrapper& value);
		ValueWrapper GetValue(std::string name);

		void SetProperty(std::string name, ValueWrapper value) override;
		ValueWrapper GetProperty(std::string name) const override;

		void SetParentScope(ValueWrapper parentScope);
	};
}