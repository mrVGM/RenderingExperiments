#pragma once

#include <string>
#include <vector>

namespace scripting
{
	struct ISymbol;
	struct SimpleSymbol;
	struct CompositeSymbol;
	struct CodeSource
	{
		std::string m_filename;
		std::string m_code;
		std::vector<ISymbol*> m_symbols;
		std::vector<ISymbol*> m_parserReady;

		void Tokenize();
		bool TokenizeForParser();
		SimpleSymbol* CreateSimpleSymbol();
		CompositeSymbol* CreateCompositeSymbol();

		~CodeSource();

	private:
		std::vector<ISymbol*> m_created;
	};
}