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
	std::map<std::string, ParsedCode>::iterator it = m_loadedCodeFiles.find(path);
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

	ParsedCode pf{ cs, parsed };
	m_loadedCodeFiles[path] = pf;
	return parsed;
}

scripting::ISymbol* interpreter::Session::ParseInstruction(std::string instruction)
{
	std::stringstream ss;
	ss << "timeout(func() {" << instruction << "}, -1);";

	scripting::CodeSource* cs = new scripting::CodeSource();
	cs->m_code = ss.str();
	cs->Tokenize();
	cs->TokenizeForParser();

	scripting::ISymbol* parsed = m_parser.Parse(*cs);
	if (!parsed) {
		delete cs;
		return nullptr;
	}

	ParsedCode pf{ cs, parsed };
	m_loadedInstructions.push_back(pf);
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

void interpreter::Session::RunInstruction(std::string instruction)
{
	scripting::ISymbol* parsed = ParseInstruction(instruction);
	m_repl = parsed;
}

void interpreter::Session::RunCallback(const Value& func, const Value& args)
{
	if (args.GetType() != ScriptingValueType::None && args.GetType() != ScriptingValueType::Object) {
		return;
	}

	if (func.GetType() != ScriptingValueType::Object) {
		return;
	}
	
	IFunc* tmp =  dynamic_cast<IFunc*>(func.GetManagedValue());
	if (!tmp) {
		return;
	}

	Callback cb;
	cb.m_func = func;
	cb.m_args = args;

	m_callbackMutex.lock();
	m_callbacks.push(cb);
	m_callbackMutex.unlock();
}

void interpreter::Session::CalculationStep()
{
	if (m_intepreterStack.empty()) {
		if (m_repl) {
			Value scope = Scope::Create();
			Scope* tmp = static_cast<Scope*>(scope.GetManagedValue());
			tmp->SetParentScope(m_motherScope);

			m_intepreterStack.push(Interpreter(scope));
			interpreter::Interpreter& interpreter = m_intepreterStack.top();
			interpreter.PrepareCalculation(m_repl);
			m_repl = nullptr;
			return;
		}

		m_callbackMutex.lock();
		if (!m_callbacks.empty()) {
			Callback cb = m_callbacks.front();
			m_callbacks.pop();

			Value scope = Scope::Create();
			Scope* tmp = static_cast<Scope*>(scope.GetManagedValue());
			tmp->BindValue("callback_func_name", cb.m_func);
			tmp->BindValue("callback_args", cb.m_args);

			Value intScope = Scope::Create();
			tmp = static_cast<Scope*>(intScope.GetManagedValue());
			tmp->SetParentScope(scope);

			m_intepreterStack.push(Interpreter(intScope));
			interpreter::Interpreter& interpreter = m_intepreterStack.top();
			interpreter.PrepareCalculation(m_callbackCallCodeParsed);

			m_callbackMutex.unlock();
			return;
		}
		m_callbackMutex.unlock();

		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

		for (std::list<DefferedCall>::const_iterator it = m_deferredCalls.begin(); it != m_deferredCalls.end(); ++it) {
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

	bool exception = top.m_state == InterpreterState::Failed;
	Value exceptionValue = top.GetException();
	if (top.m_state != InterpreterState::Pending) {
		m_intepreterStack.pop();
	}

	if (exception && !m_intepreterStack.empty()) {
		m_intepreterStack.top().HandleException(exceptionValue);
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
		bool putInFront = false;
		if (milliseconds < 0) {
			putInFront = true;
			milliseconds = 0;
		}

		DefferedCall dc;
		dc.m_scheduled = std::chrono::system_clock::now() + std::chrono::milliseconds(milliseconds);
		dc.m_func = func;

		if (putInFront) {
			m_deferredCalls.insert(m_deferredCalls.begin(), dc);
		}
		else {
			m_deferredCalls.push_back(dc);
		}
		return Value();
	});
	scope->BindValue("timeout", timeout);

	Value throwException = CreateNativeFunc(1, [](Value scope) {
		Value arg = scope.GetProperty("param0");
		scope.SetProperty("exception", arg);
		return Value();
	});
	scope->BindValue("throw", throwException);

	Value readFile = CreateNativeFunc(1, [&](Value scope) {
		Value filePathValue = scope.GetProperty("param0");
		if (filePathValue.GetType() != ScriptingValueType::String) {
			scope.SetProperty("exception", Value("Please supply a file path!"));
			return Value();
		}

		std::string fullPath = m_rootDir + filePathValue.GetString();
		std::string contents = data::GetLibrary().ReadFileByPath(fullPath);

		return Value(contents);
	});
	scope->BindValue("readFile", readFile);

	m_deferredCallCode.m_code = "func_name();";
	m_deferredCallCode.Tokenize();
	m_deferredCallCode.TokenizeForParser();
	m_deferredCallCodeParsed = m_parser.Parse(m_deferredCallCode);

	m_callbackCallCode.m_code = "callback_func_name(callback_args);";
	m_callbackCallCode.Tokenize();
	m_callbackCallCode.TokenizeForParser();
	m_callbackCallCodeParsed = m_parser.Parse(m_callbackCallCode);

	m_beginning = std::chrono::system_clock::now();

	Value now = CreateNativeFunc(1, [](Value scope) {
		std::chrono::time_point now = std::chrono::system_clock::now();
		auto nanosecs = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
		long long cnt = nanosecs.time_since_epoch().count();

		return Value(static_cast<double>(cnt) / 1000000000.0);
	});

	scope->BindValue("now", now);

	Value rootDirFunc = CreateNativeFunc(0, [&](Value scope) {
		return Value(m_rootDir);
	});

	scope->BindValue("rootDir", rootDirFunc);
}

interpreter::Session::~Session()
{
	for (std::map<std::string, ParsedCode>::iterator it = m_loadedCodeFiles.begin(); it != m_loadedCodeFiles.end(); ++it) {
		delete it->second.m_codeSource;
	}

	for (std::list<ParsedCode>::iterator it = m_loadedInstructions.begin(); it != m_loadedInstructions.end(); ++it) {
		delete it->m_codeSource;
	}
}
