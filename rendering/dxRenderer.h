#pragma once

#include "nativeObject.h"
#include "d3dx12.h"
#include <d3d12.h>
#include <dxgi1_6.h>

namespace rendering
{
	struct ISwapChain
	{
		CD3DX12_VIEWPORT m_viewport;
		CD3DX12_RECT m_scissorRect;
		virtual CD3DX12_CPU_DESCRIPTOR_HANDLE GetCurrentRTVDescriptor() const = 0;
		virtual ID3D12Resource* GetCurrentRenderTarget() const = 0;
	};

	class DXRenderer : public interpreter::INativeObject
	{
		ID3D12Device* m_device = nullptr;
		ISwapChain* m_swapChain = nullptr;
		ID3D12CommandQueue* m_commandQueue = nullptr;

		ID3D12Resource* m_dsTexture = nullptr;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		bool SetupDSVHeap(ID3D12Resource* dsTexture, std::string& errorMessage);
	public:
	};
}