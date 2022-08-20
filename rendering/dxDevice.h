#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <string>

#include "d3dx12.h"
#include "nativeObject.h"
#include "dxBuffer.h"

namespace rendering
{
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
    };

	class DXDevice : public interpreter::INativeObject
	{
        static const UINT FrameCount = 2;
        UINT m_width;
        UINT m_height;

        CD3DX12_VIEWPORT m_viewport;
        CD3DX12_RECT m_scissorRect;
        Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
        Microsoft::WRL::ComPtr<ID3D12Device> m_device;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
        UINT m_rtvDescriptorSize;

        // App resources.
        ID3D12Resource* m_vertexBuffer = nullptr;
        D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

        // Synchronization objects.
        UINT m_frameIndex;
        HANDLE m_fenceEvent;
        ID3D12Fence* m_fence = nullptr;
        UINT64 m_fenceValue;

        bool LoadPipeline(HWND hWnd, std::string& errorMessage);
        bool LoadAssets(const Microsoft::WRL::ComPtr<ID3DBlob>& vertexShader, const Microsoft::WRL::ComPtr<ID3DBlob>& pixelShader, ID3D12Fence* fence, DXBuffer* vertexBuffer, std::string& errorMessage);
        bool PopulateCommandList(std::string& errorMessage);
        bool WaitForPreviousFrame(std::string& errorMessage);

        bool Render(std::string& errorMessage);

        virtual void InitProperties(interpreter::NativeObject& nativeObject);
    public:

        ID3D12Device& GetDevice();
	};

}