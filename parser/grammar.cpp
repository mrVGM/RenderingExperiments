#include "grammar.h"

#include "codeSource.h"
#include "tokenizer.h"
#include "symbol.h"
#include "dataLib.h"


#include <functional>
#include <set>
#include <queue>
#include <cstring>

scripting::Grammar::Grammar(const std::string& grammar)
{
	CodeSource cs;
	cs.m_code = grammar;

	NewLineSymbolTokenizer newLineTokenizer;
	StringTokenizer stringTokenizer;

	cs.Tokenize();
	std::vector<ISymbol*> grammarTokens = cs.m_symbols;
	grammarTokens = newLineTokenizer.Tokenize(grammarTokens);
	grammarTokens = stringTokenizer.Tokenize(grammarTokens);

	typedef std::function<void(ISymbol*)> func;


	func notInRule, readingRHS;
	func* curFunc = &notInRule;

	Rule r;
	notInRule = [&](ISymbol* symbol) {
		if (symbol->m_name == "String") {
			r.m_lhs = symbol->m_symbolData.m_string;
			curFunc = &readingRHS;
			return;
		}
	};

	readingRHS = [&](ISymbol* symbol) {
		if (symbol->m_name == "NewLine") {
			m_rules.push_back(r);
			r.m_lhs = "";
			r.m_rhs.clear();
			curFunc = &notInRule;
			return;
		}

		if (symbol->m_name == "String") {
			r.m_rhs.push_back(symbol->m_symbolData.m_string);
		}
	};

	for (int i = 0; i < grammarTokens.size(); ++i) {
		(*curFunc)(grammarTokens[i]);
	}

	if (curFunc == &readingRHS) {
		m_rules.push_back(r);
	}
}

const std::set<std::string>& scripting::Grammar::GetTerminals()
{
	if (m_terminals.size() > 0) {
		return m_terminals;
	}

	const std::set<std::string> nonTerminals = GetNonTerminals();
	for (int i = 0; i < m_rules.size(); ++i) {
		const Rule& rule = m_rules[i];

		for (int j = 0; j < rule.m_rhs.size(); ++j) {
			const std::string& cur = rule.m_rhs[j];
			if (nonTerminals.find(cur) == nonTerminals.end()) {
				m_terminals.insert(cur);
			}
		}
	}

	return m_terminals;
}

const std::set<std::string>& scripting::Grammar::GetNonTerminals()
{
	if (m_nonTerminals.size() > 0) {
		return m_nonTerminals;
	}

	for (int i = 0; i < m_rules.size(); ++i) {
		m_nonTerminals.insert(m_rules[i].m_lhs);
	}
	
	return m_nonTerminals;
}

void scripting::Grammar::GenerateParserStates()
{
	m_parserStates.clear();

	AugmentedRule initialRule(*this, m_rules[0]);
	ParserState ps;
	ps.m_rules.push_back(initialRule);
	ps.Expand();

	m_parserStates.push_back(ps);

	std::set<int> checked;
	std::queue<int> toCheck;

	toCheck.push(0);
	while (!toCheck.empty()) {
		int curIndex = toCheck.front();
		toCheck.pop();

		if (checked.find(curIndex) != checked.end()) {
			continue;
		}
		checked.insert(curIndex);
		
		std::set<std::string> nextSymbols;
		{
			ParserState& cur = m_parserStates[curIndex];

			for (int i = 0; i < cur.m_rules.size(); ++i) {
				const AugmentedRule& rule = cur.m_rules[i];
				if (rule.m_pointPosition < rule.m_rule.m_rhs.size()) {
					nextSymbols.insert(rule.m_rule.m_rhs[rule.m_pointPosition]);
				}
			}
		}

		for (auto it = nextSymbols.begin(); it != nextSymbols.end(); ++it) {
			ParserState& cur = m_parserStates[curIndex];
			ParserState nextState = cur.ShiftWith(*it);

			bool add = true;
			for (int i = 0; i < m_parserStates.size(); ++i) {
				if (m_parserStates[i].Equals(nextState)) {
					if (m_parserStates[i].Merge(nextState)) {
						checked.erase(i);
						toCheck.push(i);
					}
					add = false;
					break;
				}
			}
			if (add) {
				m_parserStates.push_back(nextState);
				toCheck.push(m_parserStates.size() - 1);
			}
		}
	}
}

void scripting::Grammar::GetProducibleTerminals(std::set<std::string>& productions, const std::string& symbol)
{
	const std::set<std::string>& terminals = GetTerminals();
	
	std::set<std::string> checked;
	std::queue<std::string> toCheck;

	toCheck.push(symbol);

	while (!toCheck.empty())
	{
		std::string cur = toCheck.front();
		toCheck.pop();

		if (checked.find(cur) != checked.end()) {
			continue;
		}
		checked.insert(cur);

		if (terminals.find(cur) != terminals.end()) {
			productions.insert(cur);
			continue;
		}

		for (int i = 0; i < m_rules.size(); ++i) {
			const Rule& rule = m_rules[i];
			if (rule.m_lhs != cur) {
				continue;
			}

			toCheck.push(rule.m_rhs[0]);
		}
	}
}

void scripting::Grammar::GenerateParserTable(ParserTable& parserTable)
{
	std::function<int(const Rule& rule)> ruleId = [&](const Rule& rule) {
		for (int i = 0; i < m_rules.size(); ++i) {
			if (&m_rules[i] == &rule) {
				return i;
			}
		}
		return -1;
	};

	std::function<int(const ParserState& rule)> stateId = [&](const ParserState& state) {
		for (int i = 0; i < m_parserStates.size(); ++i) {
			if (m_parserStates[i].Equals(state)) {
				return i;
			}
		}
		return -1;
	};

	for (int i = 0; i < m_parserStates.size(); ++i) {
		ParserState& cur = m_parserStates[i];

		std::set<std::string> symbols;
		for (auto it = cur.m_rules.begin(); it != cur.m_rules.end(); ++it) {
			if ((*it).m_pointPosition < (*it).m_rule.m_rhs.size()) {
				symbols.insert((*it).m_rule.m_rhs[(*it).m_pointPosition]);
			}
		}

		for (auto it = symbols.begin(); it != symbols.end(); ++it) {
			ParserState ps = cur.ShiftWith(*it);
			Action a;
			a.m_origin = stateId(cur);
			a.m_target = stateId(ps);
			a.m_shift = true;
			a.m_symbol = *it;
			parserTable.m_actions.push_back(a);
		}

		for (auto it = cur.m_rules.begin(); it != cur.m_rules.end(); ++it) {
			const AugmentedRule& rule = *it;
			if (rule.m_pointPosition == rule.m_rule.m_rhs.size()) {
				Action a;
				a.m_origin = stateId(cur);
				a.m_target = ruleId(rule.m_rule);
				a.m_shift = false;

				bool anyLaSymbols = false;
				for (auto las = rule.m_lookAheadSymbols.begin(); las != rule.m_lookAheadSymbols.end(); ++las) {
					anyLaSymbols = true;
					a.m_symbol = *las;
					parserTable.m_actions.push_back(a);
				}
				if (!anyLaSymbols) {
					for (auto it = GetTerminals().begin(); it != GetTerminals().end(); ++it) {
						a.m_symbol = *it;
						parserTable.m_actions.push_back(a);
					}
					for (auto it = GetNonTerminals().begin(); it != GetNonTerminals().end(); ++it) {
						a.m_symbol = *it;
						parserTable.m_actions.push_back(a);
					}
				}
			}
		}
	}
}

void scripting::Grammar::Serialize(const char* id)
{
	int size = sizeof(int);
	for (int i = 0; i < m_rules.size(); ++i) {
		const Rule& rule = m_rules[i];
		size += rule.m_lhs.size() + 1;
		for (int j = 0; j < rule.m_rhs.size(); ++j) {
			size += rule.m_rhs[j].size() + 1;
		}
		size += 1;
	}
	--size;

	char* serialized = new char[size];

	memset(serialized, 0, size);

	int* blockSize = reinterpret_cast<int*>(serialized);
	*blockSize = size;

	char* data = reinterpret_cast<char*>(serialized);
	data += sizeof(int);
	
	for (int i = 0; i < m_rules.size(); ++i) {
		const Rule& rule = m_rules[i];
		
		memcpy(data, rule.m_lhs.c_str(), rule.m_lhs.size());
		data += rule.m_lhs.size() + 1;

		for (int j = 0; j < rule.m_rhs.size(); ++j) {
			memcpy(data, rule.m_rhs[j].c_str(), rule.m_rhs[j].size());
			data += rule.m_rhs[j].size() + 1;
		}

		data += 1;
	}

	data::GetLibrary().WriteBinFile(id, serialized, size);
	delete[] serialized;
}

void scripting::Grammar::Deserialize(const char* id)
{
	m_rules.clear();

	int size;
	data::GetLibrary().ReadBinFile(id, &size, sizeof(int));

	char* serialized = new char[size];
	data::GetLibrary().ReadBinFile(id, serialized, size);

	char* data = serialized;
	char* dataEnd = data + size;
	data += sizeof(int);


	while (data < dataEnd) {
		Rule r;

		r.m_lhs = data;
		data += r.m_lhs.size() + 1;

		while (data < dataEnd) {
			if (*data == 0) {
				data += 1;
				break;
			}
			std::string tmp = data;
			data += tmp.size() + 1;
			r.m_rhs.push_back(tmp);
		}

		m_rules.push_back(r);
	}

	delete[] serialized;
}

