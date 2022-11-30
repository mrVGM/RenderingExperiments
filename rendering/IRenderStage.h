#pragma once

#include "nativeObject.h"

#include <string>

namespace rendering
{
	class DXRenderer;

	class IRenderStage
	{
	public:
		virtual bool Init(DXRenderer& renderer, std::string& errorMessage) = 0;
		virtual bool Execute(DXRenderer& renderer, std::string& errorMessage) = 0;
	};
}