#pragma once

#include "value.h"

#include <string>
#include <ostream>

namespace interpreter
{
	struct ISession
	{
		virtual void RunFile(std::string name) = 0;
		virtual void RunInstruction(std::string instruction) = 0;
		virtual void RunFunc(const interpreter::Value& func) = 0;
		void AddGlobalValue(std::string name, const Value& value);
	};

	ISession& OpenSession(std::string scriptsDir, std::ostream& outputStream);
	ISession* GetSession();
	void CloseSession();
}