#pragma once

#include "nativeObject.h"
#include "scene/IMeshRepo.h"

#include <map>
#include <string>

namespace rendering::scene
{

	struct Mesh;
	class DXMeshRepo : public interpreter::INativeObject, public IMeshRepo
	{
		void InitProperties(interpreter::NativeObject& nativeObject) override;
	public:
	};
}