#pragma once

#include <string>
#include <ostream>

namespace interpreter
{
	struct ISession
	{
		virtual void RunFile(std::string name) = 0;
	};

	ISession& GetSession(std::string scriptsDir, std::ostream& outputStream);
	void CloseSession();
}