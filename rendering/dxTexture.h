#pragma once

#include "nativeObject.h"

#include <d3d12.h>
#include <wrl.h>
#include <string>

namespace rendering
{
	class DXTexture : public interpreter::INativeObject
	{
		DXGI_FORMAT m_format;
		UINT m_width = -1;
		UINT m_height = -1;
		UINT m_depth = -1;
		UINT m_dimension = 2;
		bool m_allowUA = false;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		bool Place(
			ID3D12Device* device,
			ID3D12Heap* heap,
			UINT64 heapOffset,
			D3D12_RESOURCE_STATES initialState,
			std::string& errorMessage);

		D3D12_RESOURCE_DESC GetTextureDescription() const;
		D3D12_RESOURCE_ALLOCATION_INFO GetTextureAllocationInfo(ID3D12Device* device);
	public:
		ID3D12Resource* GetTexture() const;
		DXGI_FORMAT GetFormat() const;
		
		UINT GetTextureWidth() const;
		UINT GetTextureHeight() const;
		UINT GetTextureDepth() const;
		UINT GetTextureDimension() const;
		UINT GetTexturePixelSize() const;
	};
}