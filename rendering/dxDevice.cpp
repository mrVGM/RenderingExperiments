#include "dxDevice.h"

#include "nativeFunc.h"
#include "window.h"

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

bool rendering::DXDevice::LoadPipeline(HWND hWnd, std::string& errorMessage)
{
    using Microsoft::WRL::ComPtr;

    UINT dxgiFactoryFlags = 0;

#if DEBUG
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    ComPtr<IDXGIFactory4> factory;
    THROW_ERROR(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)), "Can't create DXGIFactoty!")

    {
        THROW_ERROR(D3D12CreateDevice(
            nullptr,
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
        ), "Can't Create device");
    }

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    THROW_ERROR(
        m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)),
        "Can't create Command QUEUE!")

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    THROW_ERROR(factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
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
            m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)),
            "Can't create a descriptor heap!")

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            THROW_ERROR(
                m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])),
                "Can't get buffer from the Swap Chain!"
            )
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    THROW_ERROR(
        m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)),
        "Can't create Command Allocator!"
    )

    return true;
}

void rendering::DXDevice::LoadAssets()
{
}

void rendering::DXDevice::PopulateCommandList()
{
}

void rendering::DXDevice::WaitForPreviousFrame()
{
}

#undef THROW_ERROR

void rendering::DXDevice::InitProperties(interpreter::NativeObject& nativeObject)
{
	using namespace interpreter;

	Value& initProp = GetOrCreateProperty(nativeObject, "init");

	initProp = CreateNativeMethod(nativeObject, 1, [](Value scope) {
		Value self = scope.GetProperty("self");
		NativeObject* selfContainer = static_cast<NativeObject*>(self.GetManagedValue());
		DXDevice& device = static_cast<DXDevice&>(selfContainer->GetNativeObject());

		Value windowValue = scope.GetProperty("param0");
#define NO_WINDOW scope.SetProperty("exception", Value("Please supply a Window object!")); return Value();

		if (windowValue.IsNone()) {
			NO_WINDOW
		}

		NativeObject* windowContainer = dynamic_cast<NativeObject*>(windowValue.GetManagedValue());
		if (!windowContainer) {
			NO_WINDOW
		}

		Window* window = dynamic_cast<Window*>(&windowContainer->GetNativeObject());
		if (!window) {
			NO_WINDOW
		}
#undef NO_WINDOW

		device.m_width = windowContainer->GetProperty("width").GetNum();
		device.m_height = windowContainer->GetProperty("height").GetNum();

        std::string errorMessage;
        bool res = device.LoadPipeline(window->m_hwnd, errorMessage);
        if (!res) {
            self.SetProperty("exception", Value(errorMessage));
            return Value();
        }

		return Value();
	});
}
