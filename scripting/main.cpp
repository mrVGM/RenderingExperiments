// CMakeTest.cpp : Defines the entry point for the application.
//

#include "dataLib.h"
#include "grammar.h"

#include "parser.h"
#include "codeSource.h"
#include "interpreter.h"

#include <iostream>

#define DATA_LIB_DIR_TEMP "C:\\Users\\Vasil\\dev\\CMakeTest\\data\\"

using namespace std;

int main()
{
	data::Init(DATA_LIB_DIR_TEMP);

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

	cout << "Valid PT? " << validPT << endl;

	interpreter::Interpreter interpreter;
	interpreter.Calculate(s);
	return 0;
}
