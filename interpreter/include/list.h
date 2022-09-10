#pragma once

#include "value.h"

#include <list>

namespace interpreter
{
	struct ListValue : public IManagedValue
	{
		std::list<Value> m_list;

		void SetProperty(std::string name, Value value) override;
		Value GetProperty(std::string name) const override;

		Value GetValueAt(int index) const;
		void SetValueAt(int index, Value value);

		static Value Create();

		Value m_pushMethod;

	protected:
		ListValue();
	};
}