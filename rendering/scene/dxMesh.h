#pragma once

#include "nativeObject.h"
#include "scene/IMeshRepo.h"

#include <d3d12.h>
#include <list>

namespace rendering::scene
{
	class DXMesh : public interpreter::INativeObject
	{
		Mesh m_mesh;
		void InitProperties(interpreter::NativeObject& nativeObject) override;
	public:

		const Mesh& GetMesh() const;
	};
}