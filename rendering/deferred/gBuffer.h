#pragma once

#include "nativeObject.h"

#include "d3dx12.h"

#include <wrl.h>
#include <string>
#include <vector>

namespace rendering::deferred
{
	class GBuffer : public interpreter::INativeObject
	{
		ID3D12Resource* m_diffuseTex = nullptr;
		ID3D12Resource* m_normalTex = nullptr;
		ID3D12Resource* m_positionTex = nullptr;

		UINT m_rtvDescriptorSize;
		UINT m_dsvDescriptorSize;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		bool Create(
			ID3D12Device* device,
			ID3D12Resource* dsTexture,
			ID3D12Resource* diffuseTex,
			ID3D12Resource* normalTex,
			ID3D12Resource* positionTex,
			std::string& errorMessage);
	public:
		enum GBuffTextureType
		{
			GBuffer_Diffuse = 0,
			GBuffer_Normal = 1,
			GBuffer_Position = 2,
		};

		ID3D12DescriptorHeap* GetRTVHeap() const;
		ID3D12DescriptorHeap* GetDSVHeap() const;

		D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandleFor(GBuffTextureType texType);
		ID3D12Resource* GetTexture(GBuffTextureType texType);
	};
}