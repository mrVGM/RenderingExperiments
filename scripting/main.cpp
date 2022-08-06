#include "dataLib.h"
#include "ISession.h"

#include <filesystem>
#include <iostream>

int main()
{
	std::filesystem::path dataPath = std::filesystem::current_path().append("..\\..\\..\\..\\data\\");
	data::Init(dataPath.string().c_str());

	std::filesystem::path scriptsDir = dataPath.append("misc\\");

	interpreter::ISession& session = interpreter::OpenSession(scriptsDir.string(), std::cout);
	session.RunFile("test_code.txt");
	interpreter::CloseSession();

	return 0;
}
