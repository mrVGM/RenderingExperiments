#include "dataLib.h"
#include "ISession.h"

#include "api.h"
#include "utils.h"

#include "collada.h"
#include "codeSource.h"
#include "symbol.h"

#include <filesystem>
#include <iostream>

#define USE_LOCAL_DIR 1

int main(int args, const char** argv)
{
	std::string executableName = argv[0];
	int index = executableName.find_last_of('\\');

	if (index < 0) {
		std::cerr << "Can't find data directory!" << std::endl;
		return 1;
	}
	std::string executableDirectory = executableName.substr(0, index);
	std::filesystem::path executableDirPath = std::filesystem::path(executableDirectory);

#if USE_LOCAL_DIR
	std::filesystem::path dataPath = "..\\..\\..\\..\\data\\";
#else
	std::filesystem::path dataPath = executableDirPath.append("..\\data\\");
#endif

	data::Init(dataPath.string().c_str());

	collada::IColladaReader* reader = collada::GetReader();
	std::string colladaFile = dataPath.append("collada\\cube.dae").string();
	scripting::ISymbol* read = reader->ReadColladaFile(colladaFile);

	std::list<collada::ColladaNode*> rootNodes, allNodes;
	bool res = reader->ConstructColladaTree(read, rootNodes, allNodes);

	for (std::list<collada::ColladaNode*>::iterator it = allNodes.begin(); it != allNodes.end(); ++it) {
		delete *it;
	}

	return 0;


	std::filesystem::path scriptsDir = dataPath.append("scripts\\");

	interpreter::ISession& session = interpreter::OpenSession(scriptsDir.string(), std::cout);
	session.AddGlobalValue("api", rendering::GetAPI());

	const char* mainScript = "renderer.txt";
	if (args > 1) {
		mainScript = argv[1];
	}

	session.RunFile(mainScript);

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
