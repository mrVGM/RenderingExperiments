#pragma once

#include "nativeObject.h"

#include <d3d12.h>
#include <list>

namespace rendering::scene
{

	struct Mesh
	{
		ID3D12Resource* m_vertexBuffer = nullptr;
		int m_vertexBufferSize = 0;
		int m_vertexBufferStride = 0;

		ID3D12Resource* m_indexBuffer = nullptr;
		int m_indexBufferSize = 0;

		std::list<std::pair<int, int>> m_materialsMap;
	};

	class DXMesh : public interpreter::INativeObject
	{
		Mesh m_mesh;
		void InitProperties(interpreter::NativeObject& nativeObject) override;
	public:

		const Mesh& GetMesh() const;
	};
}