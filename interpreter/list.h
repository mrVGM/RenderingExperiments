#pragma once

#include "value.h"

#include <vector>

namespace interpreter
{
	struct ListValue : public IManagedValue
	{
		std::vector<Value> m_list;

		void SetProperty(std::string name, Value value) override;
		Value GetProperty(std::string name) const override;

		void PushValue(Value value);
		Value GetValueAt(int index) const;
		void SetValueAt(int index, Value valueWrapper);

		static Value Create();

	protected:
		ListValue();
	};
}