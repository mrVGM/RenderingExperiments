#pragma once

#include <string>
#include <vector>

namespace scripting
{
	struct SymbolData
	{
		float m_number;
		std::string m_string;
	};

	struct CodeSource;
	struct ISymbol
	{
		std::string m_name;
		SymbolData m_symbolData;
		CodeSource* m_codeSource = nullptr;
		virtual size_t GetCodePosition() const = 0;
	};

	struct SimpleSymbol : public ISymbol
	{
		friend struct CodeSource;
		size_t m_codePosition;
		size_t GetCodePosition() const override;
	private:
		SimpleSymbol();
	};

	struct CompositeSymbol : public ISymbol
	{
		friend struct CodeSource;
		std::vector<ISymbol*> m_childSymbols;
		size_t GetCodePosition() const override;
	private:
		CompositeSymbol();
	};
}