#pragma once

#include "ISession.h"

#include "parser.h"
#include "interpreter.h"

#include <string>

namespace interpreter
{
	struct Session : public ISession
	{
		std::string m_rootDir;
		scripting::Parser& m_parser;

		void RunFile(std::string name) override;

		Session(std::string rootDir, scripting::Parser& parser);
		~Session();
	};
}