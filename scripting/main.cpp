#include "dataLib.h"
#include "grammar.h"

#include "parser.h"
#include "codeSource.h"
#include "interpreter.h"

#include <filesystem>
#include <iostream>
#include <string>

int main()
{
	std::filesystem::path dataPath = std::filesystem::current_path().append("..\\..\\..\\..\\data\\");
	data::Init(dataPath.string().c_str());

	std::string grammar = data::GetLibrary().ReadFile("grammar");

	scripting::Grammar g(grammar);
	g.GenerateParserStates();
	scripting::ParserTable pt;
	g.GenerateParserTable(pt);
	scripting::Parser p(g, pt);

	std::string code = data::GetLibrary().ReadFile("testCode");

	scripting::CodeSource cs;
	cs.m_code = code;
	cs.Tokenize();
	cs.TokenizeForParser();
	bool validPT = pt.Validate();

	scripting::ISymbol* s =  p.Parse(cs);

	interpreter::Interpreter interpreter;
	interpreter.Calculate(s);
	return 0;
}
