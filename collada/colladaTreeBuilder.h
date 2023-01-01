#pragma once

#include "symbol.h"
#include "include/collada.h"

#include <map>
#include <list>
#include <string>
#include <stack>

namespace collada
{
	struct IColladaBuilder;
	struct ColladaTreeBuilder;

	struct ColladaBuilder
	{
		IColladaBuilder* m_builder = nullptr;
		void DoBuildStep(ColladaTreeBuilder& builder);
	};

	struct ColladaTreeBuilder
	{
		std::list<ColladaNode*>& m_generatedNodes;
		std::list<ColladaNode*> m_rootNodes;

		std::stack<ColladaBuilder> m_builders;

		bool BuildTree(scripting::ISymbol* rootSymbol);
		ColladaNode* GetNewNode();

		ColladaTreeBuilder(std::list<ColladaNode*>& nodesContainer);
	};

	struct IColladaBuilder
	{
		enum BuilderState
		{
			Pending,
			Failed,
			Done,
		};

		scripting::ISymbol* m_rootSymbol = nullptr;
		BuilderState m_state = BuilderState::Pending;
		std::list<ColladaNode*> m_generatedNodes;

		virtual void DoBuildStep(ColladaTreeBuilder& builder) = 0;
		virtual void Dispose() = 0;

	protected:
		IColladaBuilder(ColladaBuilder& colladaBuilder);
	};
}