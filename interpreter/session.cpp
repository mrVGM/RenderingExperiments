#include "session.h"

#include "codeSource.h"
#include "dataLib.h"
#include "scope.h"

struct PrintFunc : public interpreter::IFunc
{
	std::ostream& m_outputStream;
	PrintFunc(std::ostream& outputStream) :
		m_outputStream(outputStream)
	{
		m_paramNames.push_back("str");
	}

	interpreter::FuncResult Execute(interpreter::Scope& scope) override
	{
		interpreter::ValueWrapper val = scope.GetValue(m_paramNames[0]);
		m_outputStream << val.ToString() << std::endl;
		interpreter::FuncResult res;
		res.m_state = interpreter::FuncResult::Finished;
		return res;
	}
};

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
		Scope* tmp = new Scope();
		ValueWrapper scope(*tmp);
		tmp->SetParentScope(m_motherScope);

		m_intepreterStack.push(Interpreter(scope));
		interpreter::Interpreter& interpreter = m_intepreterStack.top();
		interpreter.PrepareCalculation(parsed);



		while (!m_intepreterStack.empty()) {
			interpreter::Interpreter& top = m_intepreterStack.top();
			top.CalcutateStep();

			if (top.m_state != InterpreterState::Pending) {
				m_intepreterStack.pop();
			}
		}
	}
}

interpreter::Session::Session(std::string rootDir, scripting::Parser& parser, std::ostream& outputStream) :
	m_rootDir(rootDir),
	m_parser(parser),
	m_outputStream(outputStream)
{
	Scope* sc = new Scope();
	m_motherScope = ValueWrapper(*sc);

	PrintFunc* pf = new PrintFunc(m_outputStream);
	ValueWrapper print(*pf);
	sc->BindValue("print", print);
}

interpreter::Session::~Session()
{
	for (std::map<std::string, scripting::CodeSource*>::iterator it = m_loadedCodeFiles.begin(); it != m_loadedCodeFiles.end(); ++it) {
		delete it->second;
	}
}
