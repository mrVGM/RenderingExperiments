#pragma once

#include "nativeObject.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <string>

namespace rendering
{
	class DXSwapChain : public interpreter::INativeObject
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

		bool Present(std::string& errorMessage);
		void UpdateCurrentFrameIndex();
	public:
	};
}