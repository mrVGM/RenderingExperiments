#pragma once

#include "scriptingValue.h"

#include <map>
#include <string>

namespace interpreter
{
	struct ObjectValue : public IManagedValue
	{
		std::map <std::string, ValueWrapper> m_properties;

		void SetProperty(std::string name, ValueWrapper value) override;
		ValueWrapper GetProperty(std::string name) const override;
	};
}