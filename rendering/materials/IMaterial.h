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
		Sky,
		Unlit,
		Lit,
		Cloud,
		Canvas,
	};

	struct DrawSettings
	{
		ID3D12Resource* m_vertexBuffer = nullptr;
		ID3D12Resource* m_instanceBuffer = nullptr;
		ID3D12Resource* m_indexBuffer = nullptr;

		int m_vertexBufferSize = -1;
		int m_vertexBufferStride = -1;
		int m_instanceBufferSize = -1;
		int m_instanceBufferStride = -1;
		int m_indexBufferSize = -1;
	};

	class IMaterial
	{
	public:
		virtual MaterialType GetMaterialType() const = 0;
		virtual bool Init(DXRenderer& renderer, std::string& errorMessage) = 0;
		virtual bool Render(
			rendering::DXRenderer* renderer,
			const DrawSettings& drawSettings,
			std::string& errorMessage) = 0;
	};
}