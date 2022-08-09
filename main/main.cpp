#include "dataLib.h"
#include "ISession.h"

#include "window.h"
#include "nativeFunc.h"

#include "garbageCollector.h"

#include <filesystem>
#include <iostream>
#include <thread>
#include <semaphore>

void run()
{
	rendering::Window window;

	MSG msg;

	while (true) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			if (!GetMessage(&msg, NULL, 0, 0)) {
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

int main()
{
	//std::thread t(run);

	std::filesystem::path dataPath = std::filesystem::current_path().append("..\\..\\..\\..\\data\\");
	data::Init(dataPath.string().c_str());

	std::filesystem::path scriptsDir = dataPath.append("misc\\");

	interpreter::ISession& session = interpreter::GetSession(scriptsDir.string(), std::cout);

	session.AddGlobalValue("window", interpreter::CreateNativeFunc(0, [](interpreter::Value scope) {
		volatile interpreter::GarbageCollector::GCInstructionsBatch batch;
		rendering::Window* window = new rendering::Window();
		return interpreter::Value(*window);
	}));

	session.RunFile("test_code.txt");

	std::cin.get();
	interpreter::CloseSession();
	std::cin.get();

	return 0;
}
