#include "parser.h"
#include "codeSource.h"

#include <set>
#include <stack>

scripting::Parser::Parser(Grammar& grammar,ParserTable& parserTable) :
	m_grammar(grammar),
	m_parserTable(parserTable)
{
	const std::set<std::string>& nonTerminals = m_grammar.GetNonTerminals();
	const std::set<std::string>& terminals = m_grammar.GetTerminals();

	int index = 0;
	for (auto it = nonTerminals.begin(); it != nonTerminals.end(); ++it) {
		m_symbolMap[*it] = index++;
	}

	for (auto it = terminals.begin(); it != terminals.end(); ++it) {
		m_symbolMap[*it] = index++;
	}

	m_numStates = 0;
	for (int i = 0; i < m_parserTable.m_actions.size(); ++i) {
		const Action& cur = m_parserTable.m_actions[i];
		
		if (m_numStates < cur.m_origin) {
			m_numStates = cur.m_origin;
		}

		if (cur.m_shift && m_numStates < cur.m_target) {
			m_numStates = cur.m_target;
		}
	}

	++m_numStates;

	m_table = new Action* [m_numStates];
	for (int i = 0; i < m_numStates; ++i) {
		m_table[i] = new Action[m_symbolMap.size()];
		for (int j = 0; j < m_symbolMap.size(); ++j) {
			m_table[i][j].m_origin = -1;
		}
	}

	for (int i = 0; i < m_parserTable.m_actions.size(); ++i) {
		const Action& cur = m_parserTable.m_actions[i];
		m_table[cur.m_origin][m_symbolMap[cur.m_symbol]] = cur;
	}

	m_rulesMap = new Rule * [m_grammar.m_rules.size()];
	for (int i = 0; i < m_grammar.m_rules.size(); ++i) {
		m_rulesMap[i] = &grammar.m_rules[i];
	}
}

scripting::Parser::~Parser()
{
	for (int i = 0; i < m_numStates; ++i) {
		delete[] m_table[i];
	}

	delete[] m_table;

	delete[] m_rulesMap;
}

scripting::ISymbol* scripting::Parser::Parse(CodeSource& codeSource)
{
	const std::vector<ISymbol*> src = codeSource.m_parserReady;

	std::stack<int> stateStack;
	stateStack.push(0);
	std::stack<ISymbol*> symbolStack;

	int index = 0;

	while (index < src.size()) {
		ISymbol* cur = src[index];
		int symbolCode = m_symbolMap[cur->m_name];

		int state = stateStack.top();
		const Action& action = m_table[state][symbolCode];
		if (action.m_origin < 0) {
			return nullptr;
		}

		if (action.m_shift) {
			stateStack.push(action.m_target);
			symbolStack.push(cur);
			++index;
			continue;
		}

		const Rule* rule = m_rulesMap[action.m_target];
		CompositeSymbol* tmp = codeSource.CreateCompositeSymbol();
		tmp->m_name = rule->m_lhs;

		for (int j = 0; j < rule->m_rhs.size(); ++j) {
			stateStack.pop();
			tmp->m_childSymbols.insert(tmp->m_childSymbols.begin(), symbolStack.top());
			symbolStack.pop();
		}

		const Action& shift = m_table[stateStack.top()][m_symbolMap[rule->m_lhs]];

		if (!shift.m_shift) {
			return nullptr;
		}

		stateStack.push(shift.m_target);
		symbolStack.push(tmp);
	}

	if (symbolStack.size() == 2 && symbolStack.top()->m_name == "Terminal") {
		symbolStack.pop();
		return symbolStack.top();
	}

	return nullptr;
}
