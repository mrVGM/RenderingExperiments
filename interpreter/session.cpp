#include "session.h"

#include "codeSource.h"
#include "dataLib.h"
#include "scope.h"

void interpreter::Session::RunFile(std::string name)
{
	std::string fullPath = m_rootDir + name;
	
	std::string code = data::GetLibrary().ReadFileByPath(fullPath);

	scripting::CodeSource cs;
	cs.m_code = code;
	cs.m_filename = fullPath;

	cs.Tokenize();
	cs.TokenizeForParser();

	scripting::ISymbol* parsed = m_parser.Parse(cs);

	Scope* sc = new Scope();
	ValueWrapper scope(*sc);

	PrintFunc* pf = new PrintFunc(m_outputStream);
	ValueWrapper print(*pf);
	sc->BindValue("print", print);

	Interpreter interpreter(scope);

	interpreter.PrepareCalculation(parsed);

	while (interpreter.m_state == InterpreterState::Pending) {
		interpreter.CalcutateStep();
	}
}

interpreter::Session::Session(std::string rootDir, scripting::Parser& parser, std::ostream& outputStream) :
	m_rootDir(rootDir),
	m_parser(parser),
	m_outputStream(outputStream)
{
}

interpreter::Session::~Session()
{
}
