#include "ISession.h"

#include "session.h"
#include "parser.h"
#include "dataLib.h"
#include "grammar.h"

#include <string>
#include <thread>
#include <mutex>


struct ISessionImpl : public interpreter::ISession
{
	void RunFile(std::string name) override;
	void RunInstruction(std::string instruction) override;
};

namespace _sessionData
{
	std::string m_scriptsDir;
	scripting::Grammar* m_grammar = nullptr;
	scripting::ParserTable* m_parserTable = nullptr;
	scripting::Parser* m_parser = nullptr;
	interpreter::Session* m_session = nullptr;

	bool m_running = false;
	std::mutex m_mutex;
	std::thread* m_thread = nullptr;

	ISessionImpl m_impl;

	void RunSessionThread()
	{
		m_running = true;
		while (m_running) {
			m_mutex.lock();
			m_session->CalculationStep();
			m_mutex.unlock();
		}
	}
}

void ISessionImpl::RunFile(std::string name)
{
	_sessionData::m_mutex.lock();

	_sessionData::m_session->RunFile(name);

	_sessionData::m_mutex.unlock();
}

void ISessionImpl::RunInstruction(std::string instruction)
{
	_sessionData::m_mutex.lock();

	if (!_sessionData::m_session->m_repl) {
		_sessionData::m_session->RunInstruction(instruction);
	}

	_sessionData::m_mutex.unlock();
}

interpreter::ISession& interpreter::OpenSession(std::string scriptsDir, std::ostream& outputStream)
{
	if (_sessionData::m_session) {
		return _sessionData::m_impl;
	}

	_sessionData::m_scriptsDir = scriptsDir;

	std::string grammarText = data::GetLibrary().ReadFileById("grammar");
	_sessionData::m_grammar = new scripting::Grammar(grammarText);
	_sessionData::m_grammar->GenerateParserStates();

	_sessionData::m_parserTable = new scripting::ParserTable();
	_sessionData::m_grammar->GenerateParserTable(*_sessionData::m_parserTable);

	bool validTable = _sessionData::m_parserTable->Validate();

	_sessionData::m_parser = new scripting::Parser(*_sessionData::m_grammar, *_sessionData::m_parserTable);

	_sessionData::m_session = new Session(_sessionData::m_scriptsDir, *_sessionData::m_parser, outputStream);

	_sessionData::m_thread = new std::thread(_sessionData::RunSessionThread);

	return _sessionData::m_impl;
}

void interpreter::CloseSession()
{
	_sessionData::m_running = false;
	if (_sessionData::m_thread) {
		_sessionData::m_thread->join();
		delete _sessionData::m_thread;
	}
	_sessionData::m_thread = nullptr;

	if (_sessionData::m_grammar) {
		delete _sessionData::m_grammar;
	}
	if (_sessionData::m_parserTable) {
		delete _sessionData::m_parserTable;
	}
	if (_sessionData::m_parser) {
		delete _sessionData::m_parser;
	}
	if (_sessionData::m_session) {
		delete _sessionData::m_session;
	}
	
	_sessionData::m_grammar = nullptr;
	_sessionData::m_parserTable = nullptr;
	_sessionData::m_parser = nullptr;
	_sessionData::m_session = nullptr;
}

void interpreter::ISession::AddGlobalValue(std::string name, const Value& value)
{
	_sessionData::m_mutex.lock();
	
	Value motherScope = _sessionData::m_session->m_motherScope;
	Scope* scope = static_cast<Scope*>(motherScope.GetManagedValue());
	scope->BindValue(name, value);

	_sessionData::m_mutex.unlock();
}
