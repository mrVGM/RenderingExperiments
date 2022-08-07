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
		interpreter::ValueWrapper val = scope.GetValue(m_paramNames[0]);
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

	interpreter::ValueWrapper GetScopeTemplate() override
	{
		using namespace interpreter;
		Scope* contextScope = new Scope();
		ValueWrapper context(*contextScope);

		contextScope->BindValue("running", ValueWrapper());

		interpreter::ValueWrapper argsScope = GetArgsTemplateScope();

		Scope* args = static_cast<Scope*>(argsScope.GetManagedValue());
		args->SetParentScope(context);

		return argsScope;
	}

	interpreter::FuncResult Execute(interpreter::Scope& scope) override
	{
		using namespace interpreter;

		ValueWrapper running = scope.GetProperty("running");
		if (running.IsNone()) {
			ValueWrapper path = scope.GetProperty("path");
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
			ValueWrapper runningScope(*tmp);

			ObjectValue* obj = new ObjectValue();
			ValueWrapper objValue(*obj);

			tmp->BindValue("export", objValue);
			tmp->SetParentScope(m_session.m_motherScope);

			m_session.m_intepreterStack.push(Interpreter(runningScope));
			m_session.m_intepreterStack.top().PrepareCalculation(s);

			scope.SetProperty("running", runningScope);
			return FuncResult();
		}

		ValueWrapper runningScope = scope.GetProperty("running");
		ValueWrapper exports = runningScope.GetProperty("export");

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

	RequireFunc* rf = new RequireFunc(*this);
	ValueWrapper require(*rf);
	sc->BindValue("require", require);
}

interpreter::Session::~Session()
{
	for (std::map<std::string, scripting::CodeSource*>::iterator it = m_loadedCodeFiles.begin(); it != m_loadedCodeFiles.end(); ++it) {
		delete it->second;
	}
}
