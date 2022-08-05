#pragma once

#include "scriptingValue.h"

#include <vector>

namespace interpreter
{
	struct ListValue : public IManagedValue
	{
		std::vector<ValueWrapper> m_list;

		void SetProperty(std::string name, ValueWrapper value) override;
		ValueWrapper GetProperty(std::string name) const override;

		void PushValue(ValueWrapper value);
		ValueWrapper GetValueAt(int index) const;
		void SetValueAt(int index, ValueWrapper valueWrapper);
	};
}