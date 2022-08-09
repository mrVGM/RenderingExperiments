#pragma once

#include "value.h"

#include <map>
#include <string>

namespace rendering
{
	struct RenderObject : interpreter::IManagedValue
	{
		std::map<std::string, interpreter::Value*> m_props;
		void RegisterProperty(std::string name, interpreter::Value* value);

		void SetProperty(std::string name, interpreter::Value value) override;
		interpreter::Value GetProperty(std::string name) const override;

	};
}