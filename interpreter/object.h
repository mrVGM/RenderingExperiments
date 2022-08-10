#pragma once

#include "value.h"

#include <map>
#include <string>

namespace interpreter
{
	struct ObjectValue : public IManagedValue
	{
		std::map <std::string, Value> m_properties;

		void SetProperty(std::string name, Value value) override;
		Value GetProperty(std::string name) const override;

		static Value Create();
	protected:
		ObjectValue();
	};
}