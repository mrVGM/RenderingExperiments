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
#include <chrono>
#include <vector>

namespace interpreter
{
	struct Session
	{
		struct ParsedCode
		{
			scripting::CodeSource* m_codeSource = nullptr;
			scripting::ISymbol* m_parsed = nullptr;
		};

		struct DefferedCall
		{
			std::chrono::time_point<std::chrono::system_clock> m_scheduled;
			interpreter::Value m_func;
		};

		std::chrono::time_point<std::chrono::system_clock> m_beginning;

		std::string m_rootDir;
		scripting::Parser& m_parser;
		std::ostream& m_outputStream;

		Value m_motherScope;
		std::stack<interpreter::Interpreter> m_intepreterStack;
		std::vector<DefferedCall> m_deferredCalls;
		scripting::ISymbol* m_repl = nullptr;

		std::map<std::string, ParsedCode> m_loadedCodeFiles;
		std::vector<ParsedCode> m_loadedInstructions;
		scripting::CodeSource m_deferredCallCode;
		scripting::ISymbol* m_deferredCallCodeParsed = nullptr;

		scripting::ISymbol* GetCode(std::string path);
		scripting::ISymbol* ParseInstruction(std::string instruction);

		void RunFile(std::string name);
		void RunInstruction(std::string runInstruction);
		void RunFunc(const Value& func);
		
		void CalculationStep();

		Session(std::string rootDir, scripting::Parser& parser, std::ostream& outputStream);
		~Session();
	};
}