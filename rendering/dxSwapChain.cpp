#include "dxSwapChain.h"

#include "dxDevice.h"

#include "nativeFunc.h"
#include "window.h"
#include "dxVertexShader.h"
#include "dxPixelShader.h"
#include "dxFence.h"
#include "dxCommandQueue.h"
#include "dxDevice.h"
#include "window.h"

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

bool rendering::DXSwapChain::Create(
    IDXGIFactory4* factory,
    ID3D12CommandQueue* commandQueue,
    HWND hWnd,
    ID3D12Device* device,
    int width,
    int height,
    std::string& errorMessage)
{
    using Microsoft::WRL::ComPtr;

    m_frameIndex = 0;
    m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));
    m_rtvDescriptorSize = 0;

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    THROW_ERROR(factory->CreateSwapChainForHwnd(
        commandQueue,        // Swap chain needs the queue so that it can force a flush on it.
        hWnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    ), "Can't Create Swap Chain")

    // This sample does not support fullscreen transitions.
    THROW_ERROR(
        factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER),
        "Can't Associate to Window!")

    THROW_ERROR(swapChain.As(&m_swapChain), "Can't cast to swap chain!")
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        THROW_ERROR(
            device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)),
            "Can't create a descriptor heap!")

        m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            THROW_ERROR(
                m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])),
                "Can't get buffer from the Swap Chain!")

            device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    return true;
}

bool rendering::DXSwapChain::Present(std::string& errorMessage)
{
    THROW_ERROR(m_swapChain->Present(1, 0),
        "Can't present Swap Chain!")

    return true;
}

void rendering::DXSwapChain::UpdateCurrentFrameIndex()
{
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

#undef THROW_ERROR


CD3DX12_CPU_DESCRIPTOR_HANDLE rendering::DXSwapChain::GetCurrentRTVDescriptor() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
}

ID3D12Resource* rendering::DXSwapChain::GetCurrentRenderTarget() const
{
    return m_renderTargets[m_frameIndex].Get();
}


void rendering::DXSwapChain::InitProperties(interpreter::NativeObject& nativeObject)
{
    using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

    Value& create = GetOrCreateProperty(nativeObject, "create");
    create = CreateNativeMethod(nativeObject, 3, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXSwapChain* swapChain = static_cast<DXSwapChain*>(NativeObject::ExtractNativeObject(selfValue));

        Value deviceValue = scope.GetProperty("param0");
        DXDevice* device = dynamic_cast<DXDevice*>(NativeObject::ExtractNativeObject(deviceValue));
        if (!device) {
            THROW_EXCEPTION("Please supply a device!")
        }

        Value wndValue = scope.GetProperty("param1");
        Window* wnd = dynamic_cast<Window*>(NativeObject::ExtractNativeObject(wndValue));
        if (!wnd) {
            THROW_EXCEPTION("Please supply a window!")
        }

        Value commandQueueValue = scope.GetProperty("param2");
        DXCommandQueue* commandQueue = dynamic_cast<DXCommandQueue*>(NativeObject::ExtractNativeObject(commandQueueValue));
        if (!commandQueue) {
            THROW_EXCEPTION("Please supply a command queue!")
        }

#if false
        std::string error;
        bool res = swapChain->Create(
            device->GetFactory(),
            commandQueue->GetCommandQueue(),
            wnd->m_hwnd,
            &device->GetDevice(),
            wnd->m_width,
            wnd->m_height,
            error);

        if (!res) {
            THROW_EXCEPTION(error)
        }
#endif

        return Value();
    });

    Value& present = GetOrCreateProperty(nativeObject, "present");
    present = CreateNativeMethod(nativeObject, 0, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXSwapChain* swapChain = static_cast<DXSwapChain*>(NativeObject::ExtractNativeObject(selfValue));

        std::string error;
        bool res = swapChain->Present(error);
        if (!res) {
            THROW_EXCEPTION(error)
        }
        return Value();
    });

    Value& updateCurrentFrameIndex = GetOrCreateProperty(nativeObject, "updateCurrentFrameIndex");
    updateCurrentFrameIndex = CreateNativeMethod(nativeObject, 0, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXSwapChain* swapChain = static_cast<DXSwapChain*>(NativeObject::ExtractNativeObject(selfValue));

        swapChain->UpdateCurrentFrameIndex();
        return Value();
    });

#undef THROW_EXCEPTION
}

