#pragma once

#include "nativeObject.h"

#include <list>

namespace rendering::primitives
{
	class Cube : public interpreter::INativeObject
	{
		void InitProperties(interpreter::NativeObject& nativeObject) override;
		void GenerateVertices() const;
	public:
	};
}