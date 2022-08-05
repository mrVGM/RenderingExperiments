#include "rule.h"
#include "grammar.h"

#include <functional>
#include <queue>

scripting::AugmentedRule::AugmentedRule(Grammar& grammar, const Rule& rule) :
	m_grammar(grammar),
	m_rule(rule)
{
}


scripting::AugmentedRule::AugmentedRule(const AugmentedRule& other) :
	AugmentedRule(other.m_grammar, other.m_rule)
{
	m_pointPosition = other.m_pointPosition;
	m_lookAheadSymbols = other.m_lookAheadSymbols;
}



bool scripting::AugmentedRule::Equals(const AugmentedRule& other) const
{
	if (&m_rule != &other.m_rule) {
		return false;
	}

	if (m_pointPosition != other.m_pointPosition) {
		return false;
	}

	return true;
}

bool scripting::AugmentedRule::Merge(const AugmentedRule& other)
{
	int laCount = m_lookAheadSymbols.size();
	for (auto it = other.m_lookAheadSymbols.begin(); it != other.m_lookAheadSymbols.end(); ++it) {
		m_lookAheadSymbols.insert(*it);
	}

	return laCount < m_lookAheadSymbols.size();
}

std::string scripting::AugmentedRule::ToString() const
{
	std::string res;
	res += m_rule.m_lhs + " -> ";
	bool dotPlaced = false;
	for (int i = 0; i < m_rule.m_rhs.size(); ++i) {
		if (i == m_pointPosition) {
			res += ".";
			dotPlaced = true;
		}
		res += m_rule.m_rhs[i];
	}
	if (!dotPlaced) {
		res += ".";
	}

	res += " ";

	for (auto it = m_lookAheadSymbols.begin(); it != m_lookAheadSymbols.end(); ++it) {
		res += *it;
	}
	return res;
}
