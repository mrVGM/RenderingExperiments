#pragma once

#include <vector>
#include <string>

namespace scripting
{
	struct Action
	{
		bool m_shift;
		int m_origin;
		int m_target;
		std::string m_symbol;
	};

	struct ParserTable
	{
		std::vector<Action> m_actions;
		bool Validate();

		void Serialize(const char* id);
		void Deserialize(const char* id);
	};
}