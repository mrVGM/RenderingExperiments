#include "dataLib.h"
#include "ISession.h"

#include "window.h"
#include "core.h"
#include "nativeFunc.h"

#include <filesystem>
#include <iostream>
#include <thread>
#include <semaphore>

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


	session.RunFile("test_code.txt");

	std::cin.get();
	interpreter::CloseSession();
	std::cin.get();

	return 0;
}
