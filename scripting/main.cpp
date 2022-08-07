#include "dataLib.h"
#include "ISession.h"

#include <filesystem>
#include <iostream>

#include "window.h"

int main()
{
	rendering::Window window;

	std::filesystem::path dataPath = std::filesystem::current_path().append("..\\..\\..\\..\\data\\");
	data::Init(dataPath.string().c_str());

	std::filesystem::path scriptsDir = dataPath.append("misc\\");

	interpreter::ISession& session = interpreter::OpenSession(scriptsDir.string(), std::cout);
	session.RunFile("test_code.txt");
	interpreter::CloseSession();

	std::cin.get();

	return 0;
}
