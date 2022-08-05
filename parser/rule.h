#pragma once

#include "symbol.h"

#include <string>
#include <set>

namespace scripting
{
	struct Rule
	{
		std::string m_lhs;
		std::vector<std::string> m_rhs;
	};

	class Grammar;
	struct AugmentedRule
	{
		Grammar& m_grammar;
		const Rule& m_rule;
		int m_pointPosition = 0;
		std::set<std::string> m_lookAheadSymbols;

		AugmentedRule(Grammar& grammar, const Rule& rule);
		AugmentedRule(const AugmentedRule& other);

		bool Equals(const AugmentedRule& other) const;
		bool Merge(const AugmentedRule& other);

		std::string ToString() const;
	};
}