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

        Microsoft::WRL::ComPtr<IDXGIFactory4> m_factory;

        Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
        Microsoft::WRL::ComPtr<ID3D12Device> m_device;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        UINT m_rtvDescriptorSize;

        // App resources.
        ID3D12Resource* m_vertexBuffer = nullptr;
        D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

        // Synchronization objects.
        UINT m_frameIndex;

        bool LoadPipeline(HWND hWnd, ID3D12CommandQueue* commandQueue, std::string& errorMessage);
        bool LoadAssets(DXBuffer* svertexBuffer, std::string& errorMessage);

        bool Present(std::string& errorMessage);
        bool Create(std::string& errorMessage);
        void UpdateCurrentFrameIndex();

        void InitProperties(interpreter::NativeObject& nativeObject) override;
    public:

        ID3D12Device& GetDevice();
        CD3DX12_VIEWPORT m_viewport;
        CD3DX12_RECT m_scissorRect;
        CD3DX12_CPU_DESCRIPTOR_HANDLE GetCurrentRTVDescriptor() const;
        const D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView() const;
        ID3D12Resource* GetCurrentRenderTarget() const;
        IDXGIFactory4* GetFactory() const;
	};

}