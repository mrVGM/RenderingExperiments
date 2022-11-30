#pragma once

#include "nativeObject.h"

#include <map>
#include <string>

namespace rendering::scene
{

	struct Mesh;
	class DXMeshRepo : public interpreter::INativeObject
	{
		std::map<std::string, Mesh> m_meshes;
		void InitProperties(interpreter::NativeObject& nativeObject) override;
	public:
	};
}