#include "parserState.h"

#include <functional>
#include <set>
#include <queue>
#include "grammar.h"

#include <iostream>

bool scripting::ParserState::Equals(const ParserState& other) const
{
	if (m_rules.size() != other.m_rules.size()) {
		return false;
	}

	for (int i = 0; i < m_rules.size(); ++i) {
		if (!other.HasRule(m_rules[i])) {
			return false;
		}
	}

	return true;
}

void scripting::ParserState::Expand()
{
	std::set<int> checked;
	std::queue<int> toCheck;

	for (int i = 0; i < m_rules.size(); ++i) {
		toCheck.push(i);
	}

	while (!toCheck.empty()) {
		int curIndex = toCheck.front();
		AugmentedRule& cur = m_rules[curIndex];
		toCheck.pop();
		Grammar& grammar = cur.m_grammar;
		const std::set<std::string>& terminals = grammar.GetTerminals();

		if (checked.find(curIndex) != checked.end()) {
			continue;
		}
		checked.insert(curIndex);

		if (cur.m_pointPosition == cur.m_rule.m_rhs.size()) {
			continue;
		}

		const std::string& symbol = cur.m_rule.m_rhs[cur.m_pointPosition];
		if (terminals.find(symbol) != terminals.end()) {
			continue;
		}


		std::set<std::string> laSymbols;
		int pos = cur.m_pointPosition + 1;
		if (pos < cur.m_rule.m_rhs.size()) {
			grammar.GetProducibleTerminals(laSymbols, cur.m_rule.m_rhs[pos]);
		}
		else {
			laSymbols = cur.m_lookAheadSymbols;
		}

		for (int i = 0; i < grammar.m_rules.size(); ++i) {
			const Rule& rule = grammar.m_rules[i];

			if (rule.m_lhs != symbol) {
				continue;
			}
			
			AugmentedRule ar(grammar, rule);
			ar.m_lookAheadSymbols = laSymbols;

			bool add = true;
			for (int j = 0; j < m_rules.size(); ++j) {
				if (m_rules[j].Equals(ar)) {
					if (m_rules[j].Merge(ar)) {
						checked.erase(j);
						toCheck.push(j);
					}
					add = false;
					break;
				}
			}

			if (add) {
				m_rules.push_back(ar);
				toCheck.push(m_rules.size() - 1);
			}
		}
	}
}

scripting::ParserState scripting::ParserState::ShiftWith(const std::string symbol)
{
	ParserState res;
	
	for (int i = 0; i < m_rules.size(); ++i) {
		const AugmentedRule& cur = m_rules[i];

		if (cur.m_pointPosition < cur.m_rule.m_rhs.size() &&
			cur.m_rule.m_rhs[cur.m_pointPosition] == symbol) {
			AugmentedRule tmp(cur);
			++tmp.m_pointPosition;
			res.m_rules.push_back(tmp);
		}
	}
	res.Expand();
	return res;
}

bool scripting::ParserState::Merge(const ParserState& other)
{
	bool res = false;
	for (auto i = other.m_rules.begin(); i != other.m_rules.end(); ++i) {
		for (auto j = m_rules.begin(); j != m_rules.end(); ++j) {
			if ((*j).Equals(*i)) {
				res |= (*j).Merge(*i);
			}
		}
	}
	return res;
}

std::string scripting::ParserState::ToString() const
{
	std::string res;
	for (int i = 0; i < m_rules.size(); ++i) {
		res += m_rules[i].ToString() + "\n";
	}
	return res;
}

bool scripting::ParserState::HasRule(const AugmentedRule& rule) const
{
	for (int i = 0; i < m_rules.size(); ++i) {
		if (m_rules[i].Equals(rule)) {
			return true;
		}
	}

	return false;
}
