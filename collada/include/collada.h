#pragma once

#include "symbol.h"
#include "colladaEntities.h"

#include <map>
#include <list>
#include <string>

namespace collada
{
	struct ColladaNode
	{
		scripting::ISymbol* m_rootSymbol;

		std::string m_tagName;
		std::map<std::string, std::string> m_tagProps;

		std::list<scripting::ISymbol*> m_data;

		std::list<ColladaNode*> m_children;
	};

	struct IColladaReader
	{
		virtual scripting::ISymbol* ReadColladaFile(const std::string& file) = 0;
		virtual bool ConstructColladaTree(scripting::ISymbol* rootSymbol, std::list<collada::ColladaNode*>& nodes, std::list<collada::ColladaNode*>& allNodes) = 0;
	};

	bool ConvertToScene(const std::list<collada::ColladaNode*>& nodes, Scene& scene);

	IColladaReader* GetReader();
	void ReleaseColladaReader();
}