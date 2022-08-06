#pragma once

#include <string>

namespace interpreter
{
	struct ISession
	{
		virtual void RunFile(std::string name) = 0;
	};

	ISession& OpenSession(std::string scriptsDir);
	void CloseSession();
}