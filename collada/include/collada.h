#pragma once

#include "symbol.h"

namespace collada
{
	struct IColladaReader
	{
		virtual scripting::ISymbol* ReadColladaFile(const std::string& file) = 0;
	};

	IColladaReader* GetReader();
	void ReleaseColladaReader();
}