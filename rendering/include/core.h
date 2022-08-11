#pragma once

#include "renderObject.h"
#include "value.h"

#include "d3dx12.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace rendering
{
	class Core : public RenderObject
	{
		static const UINT FrameCount = 2;
		UINT m_frameIndex = 0;

		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
		Microsoft::WRL::ComPtr<ID3D12Device> m_device;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

		CD3DX12_VIEWPORT m_viewport;
		CD3DX12_RECT m_scissorRect;

		HANDLE m_fenceEvent;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		UINT64 m_fenceValue;


		UINT m_rtvDescriptorSize;

		Core();
		void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter);
		void Init(HWND hWnd, int width, int height);
		void LoadAssets(std::string shaderFile);
		void WaitForPreviousFrame();
		void PopulateCommandList();
		void RenderFrame();

	public:
		~Core();
		static interpreter::Value Create();

		interpreter::Value m_init;
		interpreter::Value m_aspectRatio;
		interpreter::Value m_loadAssets;
		interpreter::Value m_renderFrame;
	};
}