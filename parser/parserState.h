#pragma once

#include "rule.h"

#include <vector>

namespace scripting
{
	struct ParserState
	{
		std::vector<AugmentedRule> m_rules;

		bool Equals(const ParserState& other) const;
		void Expand();

		ParserState ShiftWith(const std::string symbol);
		bool Merge(const ParserState& other);

		std::string ToString() const;
	private:
		bool HasRule(const AugmentedRule& rule) const;
	};
}