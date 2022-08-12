#include "session.h"

#include "codeSource.h"
#include "dataLib.h"
#include "scope.h"
#include "object.h"
#include "nativeFunc.h"

struct RequireFunc : public interpreter::IFunc
{
	interpreter::Session& m_session;

	interpreter::Value GetScopeTemplate() override
	{
		using namespace interpreter;
		Value context = Scope::Create();
		Scope* contextScope = static_cast<Scope*>(context.GetManagedValue());

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

			scripting::ISymbol* s = m_session.GetCode(path.GetString());
			if (!s) {
				FuncResult res;
				res.m_state = FuncResult::Failed;
				return res;
			}

			Value runningScope = Scope::Create();
			Scope* tmp = static_cast<Scope*>(runningScope.GetManagedValue());

			Value objValue = ObjectValue::Create();

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

	static interpreter::Value Create(interpreter::Session& session)
	{
		RequireFunc* req = new RequireFunc(session);
		interpreter::Value res(*req);
		return res;
	}
protected:
	RequireFunc(interpreter::Session& session) :
		m_session(session)
	{
		m_paramNames.push_back("path");
	}
};

scripting::ISymbol* interpreter::Session::GetCode(std::string path)
{
	std::map<std::string, ParsedFile>::iterator it = m_loadedCodeFiles.find(path);
	if (it != m_loadedCodeFiles.end()) {
		return it->second.m_parsed;
	}

	std::string fullPath = m_rootDir + path;

	std::string code = data::GetLibrary().ReadFileByPath(fullPath);

	scripting::CodeSource* cs = new scripting::CodeSource();
	cs->m_code = code;
	cs->m_filename = fullPath;

	cs->Tokenize();
	cs->TokenizeForParser();

	scripting::ISymbol* parsed = m_parser.Parse(*cs);
	if (!parsed) {
		delete cs;
		return nullptr;
	}

	ParsedFile pf{ cs, parsed };
	m_loadedCodeFiles[path] = pf;
	return parsed;
}

void interpreter::Session::RunFile(std::string name)
{
	scripting::ISymbol* parsed = GetCode(name);

	if (parsed) {
		Value scope = Scope::Create();
		Scope* tmp = static_cast<Scope*>(scope.GetManagedValue());
		tmp->SetParentScope(m_motherScope);

		m_intepreterStack.push(Interpreter(scope));
		interpreter::Interpreter& interpreter = m_intepreterStack.top();
		interpreter.PrepareCalculation(parsed);
	}
}

void interpreter::Session::CalculationStep()
{
	if (m_intepreterStack.empty()) {
		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

		for (std::vector<DefferedCall>::const_iterator it = m_deferredCalls.begin(); it != m_deferredCalls.end(); ++it) {
			const DefferedCall& dc = *it;
			if (dc.m_scheduled < now) {
				Value scope = Scope::Create();
				Scope* tmp = static_cast<Scope*>(scope.GetManagedValue());
				tmp->BindValue("func_name", dc.m_func);

				Value intScope = Scope::Create();
				tmp = static_cast<Scope*>(intScope.GetManagedValue());
				tmp->SetParentScope(scope);

				m_intepreterStack.push(Interpreter(intScope));
				interpreter::Interpreter& interpreter = m_intepreterStack.top();
				interpreter.PrepareCalculation(m_deferredCallCodeParsed);

				m_deferredCalls.erase(it);
				break;
			}
		}

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
	m_motherScope = Scope::Create();
	Scope* scope = static_cast<Scope*>(m_motherScope.GetManagedValue());

	Value print = CreateNativeFunc(1, [&](Value scope) {
		Value str = scope.GetProperty("param0");
		m_outputStream << str.ToString() << std::endl;
		return Value();
	});

	scope->BindValue("print", print);

	Value require = RequireFunc::Create(*this);
	scope->BindValue("require", require);

	Value timeout = CreateNativeFunc(2, [&](Value scope) {
		Value func = scope.GetProperty("param0");
		Value timeout = scope.GetProperty("param1");

		if (func.GetType() != ScriptingValueType::Object) {
			return Value();
		}
		IFunc* f = dynamic_cast<IFunc*>(func.GetManagedValue());
		if (!f) {
			return Value();
		}

		if (timeout.GetType() != ScriptingValueType::Number) {
			timeout = Value(0);
		}
		
		int milliseconds = (int)timeout.GetNum();
		if (milliseconds < 0) {
			milliseconds = 0;
		}

		DefferedCall dc;
		dc.m_scheduled = std::chrono::system_clock::now() + std::chrono::milliseconds(milliseconds);
		dc.m_func = func;

		m_deferredCalls.push_back(dc);

		return Value();
	});
	scope->BindValue("timeout", timeout);

	m_deferredCallCode.m_code = "func_name();";
	m_deferredCallCode.Tokenize();
	m_deferredCallCode.TokenizeForParser();

	m_deferredCallCodeParsed = m_parser.Parse(m_deferredCallCode);

	m_beginning = std::chrono::system_clock::now();
}

interpreter::Session::~Session()
{
	for (std::map<std::string, ParsedFile>::iterator it = m_loadedCodeFiles.begin(); it != m_loadedCodeFiles.end(); ++it) {
		delete it->second.m_codeSource;
	}
}
