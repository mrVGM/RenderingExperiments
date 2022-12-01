#pragma once

#include <d3d12.h>

namespace rendering::material
{
	enum MaterialType
	{
		Unlit,
		Lit,
	};

	class IMaterial
	{
	public:
		virtual MaterialType GetMaterialType() const = 0;
		virtual bool Render(
			ID3D12Resource* vertexBuffer,
			int vertexBufferSize,
			int vertexBufferStride,
			ID3D12Resource* indexBuffer,
			int indexBufferSize) = 0;
	};
}