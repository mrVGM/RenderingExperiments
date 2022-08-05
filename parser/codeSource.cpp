#include "codeSource.h"
#include "tokenizer.h"
#include "symbol.h"

#include <vector>
#include <iostream>

void scripting::CodeSource::Tokenize()
{
	for (int i = 0; i < m_code.size(); ++i) {
		std::string tmp;
		tmp.push_back(m_code[i]);
		SimpleSymbol* symbol = CreateSimpleSymbol();
		symbol->m_name = tmp;
		symbol->m_codePosition = i;

		m_symbols.push_back(symbol);
	}
}

void printSymbols(const std::vector<scripting::ISymbol*>& symbols)
{
	for (int i = 0; i < symbols.size(); ++i) {
		std::cout << symbols[i]->m_name << " ";
	}
}

bool scripting::CodeSource::TokenizeForParser()
{
	NewLineSymbolTokenizer newLineTokenizer;
	StringTokenizer stringTokenizer;
	CommentTokenizer commentTokenizer;
	BlankTokenizer blankTokenizer;
	UnsignedNumberTokenizer numberTokenizer;
	NameTokenizer nameTokenizer;

	std::vector<OperatorTokenizer> operatorTokenizers;
	operatorTokenizers.push_back(OperatorTokenizer("."));
	operatorTokenizers.push_back(OperatorTokenizer(","));
	operatorTokenizers.push_back(OperatorTokenizer(";"));
	operatorTokenizers.push_back(OperatorTokenizer("="));
	operatorTokenizers.push_back(OperatorTokenizer("+"));
	operatorTokenizers.push_back(OperatorTokenizer("-"));
	operatorTokenizers.push_back(OperatorTokenizer("*"));
	operatorTokenizers.push_back(OperatorTokenizer("/"));
	operatorTokenizers.push_back(OperatorTokenizer("%"));
	operatorTokenizers.push_back(OperatorTokenizer("<"));
	operatorTokenizers.push_back(OperatorTokenizer(">"));
	operatorTokenizers.push_back(OperatorTokenizer(">="));
	operatorTokenizers.push_back(OperatorTokenizer("<="));
	operatorTokenizers.push_back(OperatorTokenizer("<="));
	operatorTokenizers.push_back(OperatorTokenizer("=="));
	operatorTokenizers.push_back(OperatorTokenizer("||"));
	operatorTokenizers.push_back(OperatorTokenizer("&&"));
	operatorTokenizers.push_back(OperatorTokenizer("!"));
	operatorTokenizers.push_back(OperatorTokenizer("if"));
	operatorTokenizers.push_back(OperatorTokenizer("let"));
	operatorTokenizers.push_back(OperatorTokenizer("func"));
	operatorTokenizers.push_back(OperatorTokenizer("while"));
	operatorTokenizers.push_back(OperatorTokenizer("break"));
	operatorTokenizers.push_back(OperatorTokenizer("return"));
	operatorTokenizers.push_back(OperatorTokenizer("continue"));

	std::vector<ISymbol*> symbols = newLineTokenizer.Tokenize(m_symbols);
	if (newLineTokenizer.m_error) {
		return false;
	}
	
	symbols = stringTokenizer.Tokenize(symbols);
	if (stringTokenizer.m_error) {
		return false;
	}

	symbols = commentTokenizer.Tokenize(symbols);
	if (commentTokenizer.m_error) {
		return false;
	}

	std::vector<ISymbol*> tmp;
	for (int i = 0; i < symbols.size(); ++i) {
		if (symbols[i]->m_name == "Comment") {
			continue;
		}
		tmp.push_back(symbols[i]);
	}

	symbols = tmp;

	symbols = blankTokenizer.Tokenize(symbols);
	if (blankTokenizer.m_error) {
		return false;
	}

	for (int i = 0; i < operatorTokenizers.size(); ++i) {
		OperatorTokenizer& cur = operatorTokenizers[i];
		symbols = cur.Tokenize(symbols);
		if (cur.m_error) {
			return false;
		}
	}

	symbols = nameTokenizer.Tokenize(symbols);
	if (nameTokenizer.m_error) {
		return false;
	}

	symbols = numberTokenizer.Tokenize(symbols);
	if (numberTokenizer.m_error) {
		return false;
	}

	tmp.clear();
	
	for (int i = 0; i < symbols.size(); ++i) {
		if (symbols[i]->m_name == "Blank") {
			continue;
		}
		tmp.push_back(symbols[i]);
	}

	m_parserReady = tmp;
	SimpleSymbol* terminal = CreateSimpleSymbol();
	terminal->m_name = "Terminal";
	terminal->m_codePosition = -1;
	m_parserReady.push_back(terminal);

	return true;
}

scripting::SimpleSymbol* scripting::CodeSource::CreateSimpleSymbol()
{
	SimpleSymbol* res = new SimpleSymbol();
	res->m_codeSource = this;
	m_created.push_back(res);
	return res;
}

scripting::CompositeSymbol* scripting::CodeSource::CreateCompositeSymbol()
{
	CompositeSymbol* res = new CompositeSymbol();
	res->m_codeSource = this;
	m_created.push_back(res);
	return res;
}

scripting::CodeSource::~CodeSource()
{
	for (int i = 0; i < m_created.size(); ++i) {
		delete m_created[i];
	}
}
