#pragma once

#include "symbol.h"

#include <vector>

namespace scripting
{
	struct ITokenizer
	{
		bool m_error = false;
		virtual std::vector<ISymbol*> Tokenize(std::vector<ISymbol*>& src) = 0;
	};

	struct NewLineSymbolTokenizer : public ITokenizer
	{
		std::vector<ISymbol*> Tokenize(std::vector<ISymbol*>& src) override;
	};

	struct CommentTokenizer : public ITokenizer
	{
		std::vector<ISymbol*> Tokenize(std::vector<ISymbol*>& src) override;
	};

	struct StringTokenizer : public ITokenizer
	{
		std::vector<ISymbol*> Tokenize(std::vector<ISymbol*>& src) override;
	};

	struct BlankTokenizer : public ITokenizer
	{
		std::vector<ISymbol*> Tokenize(std::vector<ISymbol*>& src) override;
	};

	struct NumberTokenizer : public ITokenizer
	{
		std::vector<ISymbol*> Tokenize(std::vector<ISymbol*>& src) override;
	};

	struct UnsignedNumberTokenizer : public ITokenizer
	{
		std::vector<ISymbol*> Tokenize(std::vector<ISymbol*>& src) override;
	};

	struct NameTokenizer : public ITokenizer
	{
		std::vector<ISymbol*> Tokenize(std::vector<ISymbol*>& src) override;
	};

	struct KeywordTokenizer : public ITokenizer
	{
		std::string m_keyword;

		KeywordTokenizer(const std::string& keyword);
		std::vector<ISymbol*> Tokenize(std::vector<ISymbol*>& src) override;
	};
}