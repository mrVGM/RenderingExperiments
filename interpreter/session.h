#pragma once

#include "ISession.h"

#include "parser.h"
#include "interpreter.h"
#include "IFunc.h"
#include "scope.h"
#include "codeSource.h"

#include <string>
#include <ostream>
#include <map>
#include <stack>

namespace interpreter
{
	struct Session : public ISession
	{
		std::string m_rootDir;
		scripting::Parser& m_parser;
		std::ostream& m_outputStream;

		Value m_motherScope;
		std::stack<interpreter::Interpreter> m_intepreterStack;

		std::map<std::string, scripting::CodeSource*> m_loadedCodeFiles;
		scripting::CodeSource& GetCode(std::string path);

		void RunFile(std::string name) override;

		Session(std::string rootDir, scripting::Parser& parser, std::ostream& outputStream);
		~Session();
	};
}