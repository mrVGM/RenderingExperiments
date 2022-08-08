#include "session.h"

#include "codeSource.h"
#include "dataLib.h"
#include "scope.h"
#include "object.h"

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
		interpreter::Value val = scope.GetValue(m_paramNames[0]);
		m_outputStream << val.ToString() << std::endl;
		interpreter::FuncResult res;
		res.m_state = interpreter::FuncResult::Finished;
		return res;
	}
};

struct RequireFunc : public interpreter::IFunc
{
	interpreter::Session& m_session;

	RequireFunc(interpreter::Session& session) :
		m_session(session)
	{
		m_paramNames.push_back("path");
	}

	interpreter::Value GetScopeTemplate() override
	{
		using namespace interpreter;
		Scope* contextScope = new Scope();
		Value context(*contextScope);

		contextScope->BindValue("running", Value());

		interpreter::Value argsScope = GetArgsTemplateScope();

		Scope* args = static_cast<Scope*>(argsScope.GetManagedValue());
		args->SetParentScope(context);

		return argsScope;
	}

	interpreter::FuncResult Execute(interpreter::Scope& scope) override
	{
		using namespace interpreter;

		Value running = scope.GetProperty("running");
		if (running.IsNone()) {
			Value path = scope.GetProperty("path");
			if (path.GetType() != ScriptingValueType::String) {
				FuncResult res;
				res.m_state = FuncResult::Failed;
				return res;
			}

			scripting::CodeSource& cs = m_session.GetCode(path.GetString());

			scripting::ISymbol* s = m_session.m_parser.Parse(cs);
			if (!s) {
				FuncResult res;
				res.m_state = FuncResult::Failed;
				return res;
			}

			Scope* tmp = new Scope();
			Value runningScope(*tmp);

			ObjectValue* obj = new ObjectValue();
			Value objValue(*obj);

			tmp->BindValue("export", objValue);
			tmp->SetParentScope(m_session.m_motherScope);

			m_session.m_intepreterStack.push(Interpreter(runningScope));
			m_session.m_intepreterStack.top().PrepareCalculation(s);

			scope.SetProperty("running", runningScope);
			return FuncResult();
		}

		Value runningScope = scope.GetProperty("running");
		Value exports = runningScope.GetProperty("export");

		FuncResult res;
		res.m_state = FuncResult::Finished;
		res.m_returnValue = exports;
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
		Value scope(*tmp);
		tmp->SetParentScope(m_motherScope);

		m_intepreterStack.push(Interpreter(scope));
		interpreter::Interpreter& interpreter = m_intepreterStack.top();
		interpreter.PrepareCalculation(parsed);
	}
}

void interpreter::Session::CalculationStep()
{
	if (m_intepreterStack.empty()) {
		return;
	}

	interpreter::Interpreter& top = m_intepreterStack.top();
	top.CalcutateStep();

	if (top.m_state != InterpreterState::Pending) {
		m_intepreterStack.pop();
	}
}

interpreter::Session::Session(std::string rootDir, scripting::Parser& parser, std::ostream& outputStream) :
	m_rootDir(rootDir),
	m_parser(parser),
	m_outputStream(outputStream)
{
	Scope* sc = new Scope();
	m_motherScope = Value(*sc);

	PrintFunc* pf = new PrintFunc(m_outputStream);
	Value print(*pf);
	sc->BindValue("print", print);

	RequireFunc* rf = new RequireFunc(*this);
	Value require(*rf);
	sc->BindValue("require", require);
}

interpreter::Session::~Session()
{
	for (std::map<std::string, scripting::CodeSource*>::iterator it = m_loadedCodeFiles.begin(); it != m_loadedCodeFiles.end(); ++it) {
		delete it->second;
	}
}
