#pragma once

#include "nativeObject.h"

#include "d3dx12.h"
#include "dxRenderer.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <string>


namespace rendering
{
	class DXSwapChain : public interpreter::INativeObject, public ISwapChain
	{
		static const UINT FrameCount = 2;

		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
		UINT m_frameIndex;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		UINT m_rtvDescriptorSize;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[FrameCount];

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		bool Create(
			IDXGIFactory4* factory,
			ID3D12CommandQueue* commandQueue,
			HWND hWnd,
			ID3D12Device* device,
			int width,
			int height,
			std::string& errorMessage);

		
	public:
		bool Present(std::string& errorMessage) override;
		void UpdateCurrentFrameIndex() override;

		CD3DX12_CPU_DESCRIPTOR_HANDLE GetCurrentRTVDescriptor() const override;
		ID3D12Resource* GetCurrentRenderTarget() const override;
	};
}