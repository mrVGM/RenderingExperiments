#pragma once

#include "rule.h"
#include "parserState.h"

#include <string>
#include <vector>
#include <set>

#include "parserTable.h"

namespace scripting
{
	struct Grammar
	{
		std::vector<Rule> m_rules;
		std::vector<ParserState> m_parserStates;

		Grammar(const std::string& grammar);

		const std::set<std::string>& GetTerminals();
		const std::set<std::string>& GetNonTerminals();

		void GenerateParserStates();

		void GetProducibleTerminals(std::set<std::string>& terminals, const std::string& symbol);

		void GenerateParserTable(ParserTable& parserTable);

		void Serialize(const char* id);
		void Deserialize(const char* id);

	private:
		std::set<std::string> m_terminals;
		std::set<std::string> m_nonTerminals;
	};
}