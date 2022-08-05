#pragma once

#include "parserTable.h"
#include "grammar.h"
#include "symbol.h"

#include <map>

namespace scripting
{
	struct Parser
	{
		ParserTable& m_parserTable;
		Grammar& m_grammar;

		Parser(Grammar& grammar, ParserTable& parserTable);
		~Parser();

		std::map<std::string, int> m_symbolMap;
		Rule** m_rulesMap = nullptr;

		Action** m_table = nullptr;
		int m_numStates = 0;

		ISymbol* Parse(CodeSource& codeSource);
	};
}