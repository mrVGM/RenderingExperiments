#include "dataLib.h"
#include "ISession.h"

#include "api.h"
#include "utils.h"

#include <filesystem>
#include <iostream>

int main()
{
	std::filesystem::path dataPath = std::filesystem::current_path().append("..\\..\\..\\..\\data\\");
	data::Init(dataPath.string().c_str());

	std::filesystem::path scriptsDir = dataPath.append("misc\\");

	interpreter::ISession& session = interpreter::OpenSession(scriptsDir.string(), std::cout);
	session.AddGlobalValue("api", rendering::GetAPI());

	session.RunFile("main.txt");

	while (true) {
		std::cout << "> ";
		std::string instruction;
		std::getline(std::cin, instruction);

		if (instruction == "exit") {
			break;
		}
		session.RunInstruction(instruction);
	}

	interpreter::CloseSession();
	return 0;
}
