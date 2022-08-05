#include "symbol.h"

size_t scripting::SimpleSymbol::GetCodePosition() const
{
	return m_codePosition;
}

size_t scripting::CompositeSymbol::GetCodePosition() const
{
	return m_childSymbols[0]->GetCodePosition();
}

scripting::SimpleSymbol::SimpleSymbol () {}

scripting::CompositeSymbol::CompositeSymbol () {}

