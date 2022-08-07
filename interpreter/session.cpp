#include "session.h"

#include "codeSource.h"
#include "dataLib.h"
#include "scope.h"

scripting::CodeSource& interpreter::Session::GetCode(std::string path)
{
	std::map<std::string, scripting::CodeSource*>::iterator it = m_loadedCodeFiles.find(path);
	if (it != m_loadedCodeFiles.end()) {
		return *(it->second);
	}

	std::string fullPath = m_rootDir + path;

	std::string code = data::GetLibrary().ReadFileByPath(fullPath);

	scripting::CodeSource* cs = new scripting::CodeSource();
	cs->m_code = code;
	cs->m_filename = fullPath;

	cs->Tokenize();
	cs->TokenizeForParser();

	m_loadedCodeFiles[path] = cs;
	return *cs;
}

void interpreter::Session::RunFile(std::string name)
{
	scripting::CodeSource& cs = GetCode(name);
	scripting::ISymbol* parsed = m_parser.Parse(cs);

	if (parsed) {
		m_interpreter->PrepareCalculation(parsed);

		while (m_interpreter->m_state == InterpreterState::Pending) {
			m_interpreter->CalcutateStep();
		}

		m_interpreter->FreeUpResources();
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

	for (std::map<std::string, scripting::CodeSource*>::iterator it = m_loadedCodeFiles.begin(); it != m_loadedCodeFiles.end(); ++it) {
		delete it->second;
	}
}
