#include "dataLib.h"
#include "ISession.h"

#include "window.h"
#include "core.h"
#include "nativeFunc.h"

#include <filesystem>
#include <iostream>
#include <thread>
#include <semaphore>

#include <cstring>

int main()
{
	std::filesystem::path dataPath = std::filesystem::current_path().append("..\\..\\..\\..\\data\\");
	data::Init(dataPath.string().c_str());

	std::filesystem::path scriptsDir = dataPath.append("misc\\");

	interpreter::ISession& session = interpreter::GetSession(scriptsDir.string(), std::cout);


	session.AddGlobalValue("window", interpreter::CreateNativeFunc(0, [](interpreter::Value scope) {
		return rendering::Window::Create();
	}));

	session.AddGlobalValue("core", interpreter::CreateNativeFunc(0, [](interpreter::Value scope) {
		return rendering::Core::Create();
	}));

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
