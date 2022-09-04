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
		Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		bool Place(
			ID3D12Device* device,
			ID3D12Heap* heap,
			UINT64 heapOffset,
			bool allowUA,
			std::string& errorMessage);
	public:
		ID3D12Resource* GetTexture() const;
		DXGI_FORMAT GetFormat() const;
	};
}