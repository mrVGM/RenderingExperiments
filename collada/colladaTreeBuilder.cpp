#include "colladaTreeBuilder.h"

namespace 
{
#define CONSTRUCTOR_IMPL(NAME)\
NAME(collada::ColladaBuilder& colladaBuilder) :\
collada::IColladaBuilder(colladaBuilder)\
{\
}

	bool MatchSymbol(scripting::ISymbol* symbol, int numChildren, const char** childrenNames, scripting::ISymbol** childSymbols)
	{
		scripting::CompositeSymbol* cs = static_cast<scripting::CompositeSymbol*>(symbol);
		
		for (int i = 0; i < numChildren; ++i) {
			scripting::ISymbol* cur = cs->m_childSymbols[i];

			if (strcmp(cur->m_name.c_str(), childrenNames[i]) != 0) {
				return false;
			}

			childSymbols[i] = cur;
		}

		return true;
	}

	struct TagListBuilder;

	struct InitialBuilder : public collada::IColladaBuilder
	{
		CONSTRUCTOR_IMPL(InitialBuilder)

		void DoBuildStep(collada::ColladaTreeBuilder& builder) override;
		void Dispose() override;

		collada::IColladaBuilder* m_infoTagBuilder = nullptr;
		TagListBuilder* m_tagListBuilder = nullptr;
	};

	struct TagParamsBuilder;

	struct InfoTagBuilder : public collada::IColladaBuilder
	{
		CONSTRUCTOR_IMPL(InfoTagBuilder)

		TagParamsBuilder* m_tagParamsBuilder = nullptr;

		void DoBuildStep(collada::ColladaTreeBuilder& builder) override;
		void Dispose() override;
	};

	struct PropNameBuilder : public collada::IColladaBuilder
	{
		CONSTRUCTOR_IMPL(PropNameBuilder)

		std::string m_propName;
		
		PropNameBuilder* m_childBuilder = nullptr;

		void DoBuildStep(collada::ColladaTreeBuilder& builder) override;
		void Dispose() override;
	};

	struct TagParamBuilder : public collada::IColladaBuilder
	{
		CONSTRUCTOR_IMPL(TagParamBuilder)

		PropNameBuilder* m_propNameBuilder = nullptr;

		std::string m_paramName;
		std::string m_paramValue;

		void DoBuildStep(collada::ColladaTreeBuilder& builder) override;
		void Dispose() override;
	};

	struct TagParamsBuilder : public collada::IColladaBuilder
	{
		CONSTRUCTOR_IMPL(TagParamsBuilder)

		TagParamBuilder* m_tagParamBuilder = nullptr;
		TagParamsBuilder* m_childBuilder = nullptr;

		std::list<std::pair<std::string, std::string>> m_params;

		void DoBuildStep(collada::ColladaTreeBuilder& builder) override;
		void Dispose() override;
	};

	struct AnyTokenBuilder : public collada::IColladaBuilder
	{
		CONSTRUCTOR_IMPL(AnyTokenBuilder)

		scripting::ISymbol* m_symbol;

		void DoBuildStep(collada::ColladaTreeBuilder& builder) override;
		void Dispose() override;
	};

	struct AnyDataBuilder : public collada::IColladaBuilder
	{
		CONSTRUCTOR_IMPL(AnyDataBuilder)

		std::list<scripting::ISymbol*> m_symbols;

		AnyTokenBuilder* m_anyTokenBuilder = nullptr;
		AnyDataBuilder* m_childBuilder = nullptr;

		void DoBuildStep(collada::ColladaTreeBuilder& builder) override;
		void Dispose() override;
	};

	struct ClosedTagBuilder : public collada::IColladaBuilder
	{
		CONSTRUCTOR_IMPL(ClosedTagBuilder)

		TagParamsBuilder* m_tagParamsBuilder = nullptr;

		void DoBuildStep(collada::ColladaTreeBuilder& builder) override;
		void Dispose() override;
	};

	struct OpeningTagBuilder : public collada::IColladaBuilder
	{
		CONSTRUCTOR_IMPL(OpeningTagBuilder)

		TagParamsBuilder* m_tagParamsBuilder = nullptr;

		void DoBuildStep(collada::ColladaTreeBuilder& builder) override;
		void Dispose() override;
	};

	struct ClosaingTagBuilder : public collada::IColladaBuilder
	{
		CONSTRUCTOR_IMPL(ClosaingTagBuilder)

		std::string m_name;

		void DoBuildStep(collada::ColladaTreeBuilder& builder) override;
		void Dispose() override;
	};

	struct TagListBuilder;

	struct TagBuilder : public collada::IColladaBuilder
	{
		CONSTRUCTOR_IMPL(TagBuilder)

		OpeningTagBuilder* m_openingTagBuilder = nullptr;
		AnyDataBuilder* m_anyDataBuilder = nullptr;
		ClosaingTagBuilder* m_closingTagBuilder = nullptr;
		TagListBuilder* m_tagListBuilder = nullptr;

		void DoBuildStep(collada::ColladaTreeBuilder& builder) override;
		void Dispose() override;
	};

	struct AnyTagBuilder : public collada::IColladaBuilder
	{
		CONSTRUCTOR_IMPL(AnyTagBuilder)

		ClosedTagBuilder* m_closedTagBuilder = nullptr;
		TagBuilder* m_tagBuilder = nullptr;

		void DoBuildStep(collada::ColladaTreeBuilder& builder) override;
		void Dispose() override;
	};

	struct TagListBuilder : public collada::IColladaBuilder
	{
		CONSTRUCTOR_IMPL(TagListBuilder)

		AnyTagBuilder* m_anyTagBuilder = nullptr;
		TagListBuilder* m_childBuilder = nullptr;

		void DoBuildStep(collada::ColladaTreeBuilder& builder) override;
		void Dispose() override;
	};

#undef CONSTRUCTOR_IMPL


	void InitialBuilder::DoBuildStep(collada::ColladaTreeBuilder& builder)
	{
		const char* childNames[] = { "InfoTag", "TagList" };
		scripting::ISymbol* childSymbols[_countof(childNames)];

		if (!MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols)) {
			m_state = BuilderState::Failed;
			return;
		}

		if (!m_infoTagBuilder) {
			builder.m_builders.push(collada::ColladaBuilder());
			collada::ColladaBuilder& cb = builder.m_builders.top();
			m_infoTagBuilder = new InfoTagBuilder(cb);
			m_infoTagBuilder->m_rootSymbol = childSymbols[0];
			return;
		}

		if (m_infoTagBuilder->m_state == BuilderState::Failed) {
			m_state = BuilderState::Failed;
			return;
		}

		if (!m_tagListBuilder) {
			builder.m_builders.push(collada::ColladaBuilder());
			collada::ColladaBuilder& cb = builder.m_builders.top();
			m_tagListBuilder = new TagListBuilder(cb);
			m_tagListBuilder->m_rootSymbol = childSymbols[1];
			return;
		}

		if (m_tagListBuilder->m_state == BuilderState::Failed) {
			m_state = BuilderState::Failed;
			return;
		}

		m_generatedNodes.push_back(*m_infoTagBuilder->m_generatedNodes.begin());
		for (std::list<collada::ColladaNode*>::iterator it = m_tagListBuilder->m_generatedNodes.begin();
			it != m_tagListBuilder->m_generatedNodes.end(); ++it) {
			m_generatedNodes.push_back(*it);
		}

		m_state = BuilderState::Done;
	}

	void InitialBuilder::Dispose()
	{
		if (m_infoTagBuilder) {
			delete m_infoTagBuilder;
		}
		if (m_tagListBuilder) {
			delete m_tagListBuilder;
		}

		m_infoTagBuilder = nullptr;
		m_tagListBuilder = nullptr;
	}

	void InfoTagBuilder::DoBuildStep(collada::ColladaTreeBuilder& builder)
	{
		{
			const char* childNames[] = { "<", "?", "Name", "?", ">" };
			scripting::ISymbol* childSymbols[_countof(childNames)];

			if (MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols)) {
				collada::ColladaNode* cn = builder.GetNewNode();
				cn->m_rootSymbol = m_rootSymbol;
				cn->m_tagName = childSymbols[2]->m_symbolData.m_string;
				m_generatedNodes.push_back(cn);

				m_state = BuilderState::Done;
				return;
			}
		}

		{
			const char* childNames[] = { "<", "?", "Name", "TagParams", "?", ">" };
			scripting::ISymbol* childSymbols[_countof(childNames)];

			if (MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols)) {

				if (!m_tagParamsBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_tagParamsBuilder = new TagParamsBuilder(cb);
					m_tagParamsBuilder->m_rootSymbol = childSymbols[3];
					return;
				}

				if (m_tagParamsBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}

				collada::ColladaNode* cn = builder.GetNewNode();
				cn->m_rootSymbol = m_rootSymbol;
				cn->m_tagName = childSymbols[2]->m_symbolData.m_string;
				
				for (std::list<std::pair<std::string, std::string>>::iterator it = m_tagParamsBuilder->m_params.begin();
					it != m_tagParamsBuilder->m_params.end(); ++it) {
					cn->m_tagProps.insert(*it);
				}

				m_generatedNodes.push_back(cn);

				m_state = BuilderState::Done;
				return;
			}
		}
		

		m_state = BuilderState::Failed;
	}

	void InfoTagBuilder::Dispose()
	{
		if (m_tagParamsBuilder) {
			delete m_tagParamsBuilder;
		}

		m_tagParamsBuilder = nullptr;
	}

	void PropNameBuilder::DoBuildStep(collada::ColladaTreeBuilder& builder)
	{
		{
			const char* childNames[] = { "Name" };
			scripting::ISymbol* childSymbols[_countof(childNames)];

			if (MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols)) {
				m_propName = childSymbols[0]->m_symbolData.m_string;

				m_state = BuilderState::Done;
				return;
			}
		}

		{
			const char* childNames[] = { "PropName", ":", "Name" };
			scripting::ISymbol* childSymbols[_countof(childNames)];

			if (MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols)) {
				
				if (!m_childBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_childBuilder = new PropNameBuilder(cb);
					m_childBuilder->m_rootSymbol = childSymbols[0];
					return;
				}

				if (m_childBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}

				m_propName = m_childBuilder->m_propName + ":" + childSymbols[2]->m_symbolData.m_string;
				m_state = BuilderState::Done;
				return;
			}
		}

		m_state = BuilderState::Failed;
	}

	void PropNameBuilder::Dispose()
	{
		if (m_childBuilder) {
			delete m_childBuilder;
		}
		m_childBuilder = nullptr;
	}

	void TagParamBuilder::DoBuildStep(collada::ColladaTreeBuilder& builder)
	{
		const char* childNames[] = { "PropName", "=", "String" };
		scripting::ISymbol* childSymbols[_countof(childNames)];

		MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols);

		if (!m_propNameBuilder) {
			builder.m_builders.push(collada::ColladaBuilder());
			collada::ColladaBuilder& cb = builder.m_builders.top();
			m_propNameBuilder = new PropNameBuilder(cb);
			m_propNameBuilder->m_rootSymbol = childSymbols[0];
			return;
		}

		if (m_propNameBuilder->m_state == BuilderState::Failed) {
			m_state = BuilderState::Failed;
			return;
		}

		m_paramName = m_propNameBuilder->m_propName;
		m_paramValue = childSymbols[2]->m_symbolData.m_string;

		m_state = BuilderState::Done;	
	}

	void TagParamBuilder::Dispose()
	{
		if (m_propNameBuilder) {
			delete m_propNameBuilder;
		}

		m_propNameBuilder = nullptr;
	}
	
	void TagParamsBuilder::DoBuildStep(collada::ColladaTreeBuilder& builder)
	{
		{
			const char* childNames[] = { "TagParam" };
			scripting::ISymbol* childSymbols[_countof(childNames)];

			if (MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols)) {
				if (!m_tagParamBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_tagParamBuilder = new TagParamBuilder(cb);
					m_tagParamBuilder->m_rootSymbol = childSymbols[0];
					return;
				}

				if (m_tagParamBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}

				m_params.push_back(std::pair<std::string, std::string>(m_tagParamBuilder->m_paramName, m_tagParamBuilder->m_paramValue));

				m_state = BuilderState::Done;
				return;
			}
		}

		{
			const char* childNames[] = { "TagParams", "TagParam" };
			scripting::ISymbol* childSymbols[_countof(childNames)];

			if (MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols)) {
				if (!m_childBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_childBuilder = new TagParamsBuilder(cb);
					m_childBuilder->m_rootSymbol = childSymbols[0];
					return;
				}

				if (m_childBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}

				if (!m_tagParamBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_tagParamBuilder = new TagParamBuilder(cb);
					m_tagParamBuilder->m_rootSymbol = childSymbols[1];
					return;
				}

				if (m_tagParamBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}

				m_params = m_childBuilder->m_params;
				m_params.push_back(std::pair<std::string, std::string>(m_tagParamBuilder->m_paramName, m_tagParamBuilder->m_paramValue));

				m_state = BuilderState::Done;
				return;
			}
		}

		m_state = BuilderState::Failed;
	}

	void TagParamsBuilder::Dispose()
	{
		if (m_childBuilder) {
			delete m_childBuilder;
		}

		if (m_tagParamBuilder) {
			delete m_tagParamBuilder;
		}

		m_childBuilder = nullptr;
		m_tagParamBuilder = nullptr;
	}

	void AnyTokenBuilder::DoBuildStep(collada::ColladaTreeBuilder& builder)
	{
		scripting::CompositeSymbol* cs = static_cast<scripting::CompositeSymbol*>(m_rootSymbol);

		m_symbol = cs->m_childSymbols[0];
		m_state = BuilderState::Done;
	}

	void AnyTokenBuilder::Dispose()
	{
	}

	void AnyDataBuilder::DoBuildStep(collada::ColladaTreeBuilder& builder)
	{
		{
			const char* childNames[] = { "AnyToken" };
			scripting::ISymbol* childSymbols[_countof(childNames)];

			if (MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols)) {
				if (!m_anyTokenBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_anyTokenBuilder = new AnyTokenBuilder(cb);
					m_anyTokenBuilder->m_rootSymbol = childSymbols[0];
					return;
				}

				if (m_anyTokenBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}

				m_symbols.push_back(m_anyTokenBuilder->m_symbol);

				m_state = BuilderState::Done;
				return;
			}
		}

		{
			const char* childNames[] = { "AnyData", "AnyToken" };
			scripting::ISymbol* childSymbols[_countof(childNames)];

			if (MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols)) {
				if (!m_anyTokenBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_anyTokenBuilder = new AnyTokenBuilder(cb);
					m_anyTokenBuilder->m_rootSymbol = childSymbols[1];
					return;
				}

				if (m_anyTokenBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}

				if (!m_childBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_childBuilder = new AnyDataBuilder(cb);
					m_childBuilder->m_rootSymbol = childSymbols[0];
					return;
				}

				if (m_childBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}

				m_symbols = m_childBuilder->m_symbols;
				m_symbols.push_back(m_anyTokenBuilder->m_symbol);

				m_state = BuilderState::Done;
				return;
			}
		}

		m_state = BuilderState::Failed;
	}
	
	void AnyDataBuilder::Dispose()
	{
		if (m_anyTokenBuilder) {
			delete m_anyTokenBuilder;
		}

		if (m_childBuilder) {
			delete m_childBuilder;
		}

		m_anyTokenBuilder = nullptr;
		m_childBuilder = nullptr;
	}

	void ClosedTagBuilder::DoBuildStep(collada::ColladaTreeBuilder& builder)
	{
		{
			const char* childNames[] = { "<", "Name", "/", ">" };
			scripting::ISymbol* childSymbols[_countof(childNames)];

			if (MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols)) {
				collada::ColladaNode* cn = builder.GetNewNode();
				cn->m_rootSymbol = m_rootSymbol;
				cn->m_tagName = childSymbols[1]->m_symbolData.m_string;
				m_generatedNodes.push_back(cn);

				m_state = BuilderState::Done;
				return;
			}
		}

		{
			const char* childNames[] = { "<", "Name", "TagParams", "/", ">" };
			scripting::ISymbol* childSymbols[_countof(childNames)];

			if (MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols)) {

				if (!m_tagParamsBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_tagParamsBuilder = new TagParamsBuilder(cb);
					m_tagParamsBuilder->m_rootSymbol = childSymbols[2];
					return;
				}

				if (m_tagParamsBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}

				collada::ColladaNode* cn = builder.GetNewNode();
				cn->m_rootSymbol = m_rootSymbol;
				cn->m_tagName = childSymbols[1]->m_symbolData.m_string;

				for (std::list<std::pair<std::string, std::string>>::iterator it = m_tagParamsBuilder->m_params.begin();
					it != m_tagParamsBuilder->m_params.end(); ++it) {
					cn->m_tagProps.insert(*it);
				}

				m_generatedNodes.push_back(cn);

				m_state = BuilderState::Done;
				return;
			}
		}


		m_state = BuilderState::Failed;
	}

	void ClosedTagBuilder::Dispose()
	{
		if (m_tagParamsBuilder) {
			delete m_tagParamsBuilder;
		}

		m_tagParamsBuilder = nullptr;
	}
	
	void OpeningTagBuilder::DoBuildStep(collada::ColladaTreeBuilder& builder)
	{
		{
			const char* childNames[] = { "<", "Name", ">" };
			scripting::ISymbol* childSymbols[_countof(childNames)];

			if (MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols)) {
				collada::ColladaNode* cn = builder.GetNewNode();
				cn->m_rootSymbol = m_rootSymbol;
				cn->m_tagName = childSymbols[1]->m_symbolData.m_string;
				m_generatedNodes.push_back(cn);

				m_state = BuilderState::Done;
				return;
			}
		}

		{
			const char* childNames[] = { "<", "Name", "TagParams", ">" };
			scripting::ISymbol* childSymbols[_countof(childNames)];

			if (MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols)) {

				if (!m_tagParamsBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_tagParamsBuilder = new TagParamsBuilder(cb);
					m_tagParamsBuilder->m_rootSymbol = childSymbols[2];
					return;
				}

				if (m_tagParamsBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}

				collada::ColladaNode* cn = builder.GetNewNode();
				cn->m_rootSymbol = m_rootSymbol;
				cn->m_tagName = childSymbols[1]->m_symbolData.m_string;

				for (std::list<std::pair<std::string, std::string>>::iterator it = m_tagParamsBuilder->m_params.begin();
					it != m_tagParamsBuilder->m_params.end(); ++it) {
					cn->m_tagProps.insert(*it);
				}

				m_generatedNodes.push_back(cn);

				m_state = BuilderState::Done;
				return;
			}
		}


		m_state = BuilderState::Failed;
	}
	
	void OpeningTagBuilder::Dispose()
	{
		if (m_tagParamsBuilder) {
			delete m_tagParamsBuilder;
		}

		m_tagParamsBuilder = nullptr;
	}
	
	void ClosaingTagBuilder::DoBuildStep(collada::ColladaTreeBuilder& builder)
	{
		{
			const char* childNames[] = { "<", "/", "Name", ">" };
			scripting::ISymbol* childSymbols[_countof(childNames)];

			if (MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols)) {
				m_name = childSymbols[2]->m_symbolData.m_string;
				
				m_state = BuilderState::Done;
				return;
			}
		}

		m_state = BuilderState::Failed;
	}
	void ClosaingTagBuilder::Dispose()
	{
	}
	
	void TagBuilder::DoBuildStep(collada::ColladaTreeBuilder& builder)
	{
		{
			const char* childNames[] = { "OpeningTag", "AnyData", "ClosingTag" };
			scripting::ISymbol* childSymbols[_countof(childNames)];

			if (MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols)) {
				if (!m_closingTagBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_closingTagBuilder = new ClosaingTagBuilder(cb);
					m_closingTagBuilder->m_rootSymbol = childSymbols[2];
					return;
				}

				if (m_closingTagBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}

				if (!m_anyDataBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_anyDataBuilder = new AnyDataBuilder(cb);
					m_anyDataBuilder->m_rootSymbol = childSymbols[1];
					return;
				}

				if (m_anyDataBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}

				if (!m_openingTagBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_openingTagBuilder = new OpeningTagBuilder(cb);
					m_openingTagBuilder->m_rootSymbol = childSymbols[0];
					return;
				}

				if (m_openingTagBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}

				collada::ColladaNode* node = *m_openingTagBuilder->m_generatedNodes.begin();
				
				if (node->m_tagName != m_closingTagBuilder->m_name) {
					m_state = BuilderState::Failed;
					return;
				}

				node->m_data = m_anyDataBuilder->m_symbols;
				m_generatedNodes.push_back(node);

				m_state = BuilderState::Done;
				return;
			}
		}

		{
			const char* childNames[] = { "OpeningTag", "TagList", "ClosingTag" };
			scripting::ISymbol* childSymbols[_countof(childNames)];

			if (MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols)) {
				if (!m_closingTagBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_closingTagBuilder = new ClosaingTagBuilder(cb);
					m_closingTagBuilder->m_rootSymbol = childSymbols[2];
					return;
				}

				if (m_closingTagBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}

				if (!m_tagListBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_tagListBuilder = new TagListBuilder(cb);
					m_tagListBuilder->m_rootSymbol = childSymbols[1];
					return;
				}

				if (m_tagListBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}

				if (!m_openingTagBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_openingTagBuilder = new OpeningTagBuilder(cb);
					m_openingTagBuilder->m_rootSymbol = childSymbols[0];
					return;
				}

				if (m_openingTagBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}

				collada::ColladaNode* node = *m_openingTagBuilder->m_generatedNodes.begin();

				if (node->m_tagName != m_closingTagBuilder->m_name) {
					m_state = BuilderState::Failed;
					return;
				}

				node->m_children = m_tagListBuilder->m_generatedNodes;
				m_generatedNodes.push_back(node);

				m_state = BuilderState::Done;
				return;
			}
		}

		m_state = BuilderState::Failed;

	}

	void TagBuilder::Dispose()
	{
		if (m_openingTagBuilder) {
			delete m_openingTagBuilder;
		}

		if (m_anyDataBuilder) {
			delete m_anyDataBuilder;
		}

		if (m_closingTagBuilder) {
			delete m_closingTagBuilder;
		}

		if (m_tagListBuilder) {
			delete m_tagListBuilder;
		}

		m_openingTagBuilder = nullptr;
		m_anyDataBuilder = nullptr;
		m_closingTagBuilder = nullptr;
		m_tagListBuilder = nullptr;
	}

	void AnyTagBuilder::DoBuildStep(collada::ColladaTreeBuilder& builder)
	{
		{
			const char* childNames[] = { "ClosedTag" };
			scripting::ISymbol* childSymbols[_countof(childNames)];

			if (MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols)) {
				if (!m_closedTagBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_closedTagBuilder = new ClosedTagBuilder(cb);
					m_closedTagBuilder->m_rootSymbol = childSymbols[0];
					return;
				}

				if (m_closedTagBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}

				m_generatedNodes.push_back(*m_closedTagBuilder->m_generatedNodes.begin());

				m_state = BuilderState::Done;
				return;
			}
		}

		{
			const char* childNames[] = { "Tag" };
			scripting::ISymbol* childSymbols[_countof(childNames)];

			if (MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols)) {
				if (!m_tagBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_tagBuilder = new TagBuilder(cb);
					m_tagBuilder->m_rootSymbol = childSymbols[0];
					return;
				}

				if (m_tagBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}

				m_generatedNodes.push_back(*m_tagBuilder->m_generatedNodes.begin());

				m_state = BuilderState::Done;
				return;
			}
		}

		m_state = BuilderState::Failed;
	}
	
	void AnyTagBuilder::Dispose()
	{
		if (m_closedTagBuilder) {
			delete m_closedTagBuilder;
		}

		if (m_tagBuilder) {
			delete m_tagBuilder;
		}

		m_closedTagBuilder = nullptr;
		m_tagBuilder = nullptr;
	}
	
	void TagListBuilder::DoBuildStep(collada::ColladaTreeBuilder& builder)
	{
		{
			const char* childNames[] = { "AnyTag" };
			scripting::ISymbol* childSymbols[_countof(childNames)];

			if (MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols)) {
				if (!m_anyTagBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_anyTagBuilder = new AnyTagBuilder(cb);
					m_anyTagBuilder->m_rootSymbol = childSymbols[0];
					return;
				}

				if (m_anyTagBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}

				m_generatedNodes.push_back(*m_anyTagBuilder->m_generatedNodes.begin());

				m_state = BuilderState::Done;
				return;
			}
		}

		{
			const char* childNames[] = { "TagList", "AnyTag" };
			scripting::ISymbol* childSymbols[_countof(childNames)];

			if (MatchSymbol(m_rootSymbol, _countof(childNames), childNames, childSymbols)) {
				if (!m_childBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_childBuilder = new TagListBuilder(cb);
					m_childBuilder->m_rootSymbol = childSymbols[0];
					return;
				}

				if (m_childBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}
				
				if (!m_anyTagBuilder) {
					builder.m_builders.push(collada::ColladaBuilder());
					collada::ColladaBuilder& cb = builder.m_builders.top();
					m_anyTagBuilder = new AnyTagBuilder(cb);
					m_anyTagBuilder->m_rootSymbol = childSymbols[1];
					return;
				}

				if (m_anyTagBuilder->m_state == BuilderState::Failed) {
					m_state = BuilderState::Failed;
					return;
				}

				m_generatedNodes = m_childBuilder->m_generatedNodes;
				m_generatedNodes.push_back(*m_anyTagBuilder->m_generatedNodes.begin());

				m_state = BuilderState::Done;
				return;
			}
		}

		m_state = BuilderState::Failed;
	}

	void TagListBuilder::Dispose()
	{
		if (m_anyTagBuilder) {
			delete m_anyTagBuilder;
		}

		if (m_childBuilder) {
			delete m_childBuilder;
		}

		m_anyTagBuilder = nullptr;
		m_childBuilder = nullptr;
	}
}


collada::IColladaBuilder::IColladaBuilder(collada::ColladaBuilder& colladaBuilder)
{
	colladaBuilder.m_builder = this;
}

void collada::ColladaBuilder::DoBuildStep(ColladaTreeBuilder& builder)
{
	m_builder->DoBuildStep(builder);
}

bool collada::ColladaTreeBuilder::BuildTree(scripting::ISymbol* rootSymbol)
{
	m_builders.push(ColladaBuilder());
	ColladaBuilder& initial = m_builders.top();

	InitialBuilder* initialBuider = new InitialBuilder(initial);
	initialBuider->m_rootSymbol = rootSymbol;

	bool success = true;

	while(!m_builders.empty()) {
		ColladaBuilder& cur = m_builders.top();

		if (cur.m_builder->m_state == IColladaBuilder::BuilderState::Pending) {
			cur.DoBuildStep(*this);
			continue;
		}

		if (cur.m_builder->m_state == IColladaBuilder::BuilderState::Done) {
			cur.m_builder->Dispose();
			m_builders.pop();
			continue;
		}
		
		while (!m_builders.empty()) {
			success = false;
			m_builders.top().m_builder->Dispose();
			m_builders.pop();
		}

		break;
	}

	m_rootNodes = initialBuider->m_generatedNodes;
	delete initialBuider;

	return success;
}

collada::ColladaNode* collada::ColladaTreeBuilder::GetNewNode()
{
	ColladaNode* cn = new ColladaNode();
	m_generatedNodes.push_back(cn);

	return cn;
}

collada::ColladaTreeBuilder::ColladaTreeBuilder(std::list<ColladaNode*>& nodesContainer) :
	m_generatedNodes(nodesContainer)
{
}
