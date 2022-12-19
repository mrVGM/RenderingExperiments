#pragma once

#include <d3d12.h>
#include <string>

namespace rendering
{
	class DXRenderer;
}

namespace rendering::material
{
	enum MaterialType
	{
		Unlit,
		Lit,
		Cloud,
		Canvas,
	};

	class IMaterial
	{
	public:
		virtual MaterialType GetMaterialType() const = 0;
		virtual bool Init(DXRenderer& renderer, std::string& errorMessage) = 0;
		virtual bool Render(
			rendering::DXRenderer* renderer,

			ID3D12Resource* vertexBuffer,
			int vertexBufferSize,
			int vertexBufferStride,

			ID3D12Resource* instanceBuffer,
			int instanceBufferSize,
			int instanceBufferStride,

			ID3D12Resource* indexBuffer,
			int indexBufferSize,
			std::string& errorMessage) = 0;
	};
}