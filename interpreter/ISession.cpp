#include "ISession.h"

#include "session.h"
#include "parser.h"
#include "dataLib.h"
#include "grammar.h"

#include <string>

namespace _sessionData
{
	std::string m_scriptsDir;
	scripting::Grammar* m_grammar = nullptr;
	scripting::ParserTable* m_parserTable = nullptr;
	scripting::Parser* m_parser = nullptr;
	interpreter::Session* m_session = nullptr;
}

interpreter::ISession& interpreter::OpenSession(std::string scriptsDir)
{
	if (_sessionData::m_session) {
		return *_sessionData::m_session;
	}

	_sessionData::m_scriptsDir = scriptsDir;

	std::string grammarText = data::GetLibrary().ReadFileById("grammar");
	_sessionData::m_grammar = new scripting::Grammar(grammarText);
	_sessionData::m_grammar->GenerateParserStates();

	_sessionData::m_parserTable = new scripting::ParserTable();
	_sessionData::m_grammar->GenerateParserTable(*_sessionData::m_parserTable);

	_sessionData::m_parser = new scripting::Parser(*_sessionData::m_grammar, *_sessionData::m_parserTable);

	_sessionData::m_session = new Session(_sessionData::m_scriptsDir, *_sessionData::m_parser);
	return *_sessionData::m_session;
}

void interpreter::CloseSession()
{
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
