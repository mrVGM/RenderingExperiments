#pragma once

#include "nativeObject.h"

#include "d3dx12.h"

#include <wrl.h>
#include <string>

namespace rendering::deferred
{
	class GBuffer : public interpreter::INativeObject
	{
		UINT m_rtvDescriptorSize;
		UINT m_dsvDescriptorSize;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		bool Create(
			ID3D12Device* device,
			ID3D12Resource* dsTexture,
			ID3D12Resource* diffuseTex,
			std::string& errorMessage);
	public:
		ID3D12DescriptorHeap* GetRTVHeap() const;
		ID3D12DescriptorHeap* GetDSVHeap() const;
	};
}