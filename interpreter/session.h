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

namespace interpreter
{
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

	struct Session : public ISession
	{
		std::string m_rootDir;
		scripting::Parser& m_parser;
		std::ostream& m_outputStream;
		interpreter::Interpreter* m_interpreter = nullptr;

		std::map<std::string, scripting::CodeSource*> m_loadedCodeFiles;
		scripting::CodeSource& GetCode(std::string path);

		void RunFile(std::string name) override;

		Session(std::string rootDir, scripting::Parser& parser, std::ostream& outputStream);
		~Session();
	};
}