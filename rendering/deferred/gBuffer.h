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
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		bool Create(
			ID3D12Device* device,
			ID3D12Resource* diffuseTex,
			std::string& errorMessage);
	public:
		ID3D12DescriptorHeap* GetRTVHeap() const;
	};
}