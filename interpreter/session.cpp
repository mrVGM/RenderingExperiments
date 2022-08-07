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

	if (parsed) {
		m_interpreter->PrepareCalculation(parsed);

		while (m_interpreter->m_state == InterpreterState::Pending) {
			m_interpreter->CalcutateStep();
		}
	}
}

interpreter::Session::Session(std::string rootDir, scripting::Parser& parser, std::ostream& outputStream) :
	m_rootDir(rootDir),
	m_parser(parser),
	m_outputStream(outputStream)
{
	Scope* sc = new Scope();
	ValueWrapper scope(*sc);

	PrintFunc* pf = new PrintFunc(m_outputStream);
	ValueWrapper print(*pf);
	sc->BindValue("print", print);

	m_interpreter = new interpreter::Interpreter(scope);
}

interpreter::Session::~Session()
{
	delete m_interpreter;
}
