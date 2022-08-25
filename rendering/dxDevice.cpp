#include "dxDevice.h"

#include "nativeFunc.h"
#include "window.h"
#include "dxVertexShader.h"
#include "dxPixelShader.h"
#include "dxFence.h"
#include "dxCommandQueue.h"

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}


bool rendering::DXDevice::Create(std::string& errorMessage)
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

    THROW_ERROR(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)), "Can't create DXGIFactoty!")

    {
        THROW_ERROR(D3D12CreateDevice(
            nullptr,
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
        ), "Can't Create device");
    }

    return true;
}

void rendering::DXDevice::UpdateCurrentFrameIndex()
{
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

bool rendering::DXDevice::LoadPipeline(HWND hWnd, ID3D12CommandQueue* commandQueue, std::string & errorMessage)
{
    using Microsoft::WRL::ComPtr;

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
    THROW_ERROR(m_factory->CreateSwapChainForHwnd(
        commandQueue,        // Swap chain needs the queue so that it can force a flush on it.
        hWnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    ), "Can't Create Swap Chain")

    // This sample does not support fullscreen transitions.
    THROW_ERROR(
        m_factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER),
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

    return true;
}

bool rendering::DXDevice::LoadAssets(DXBuffer* vertexBuffer, std::string& errorMessage)
{
    using Microsoft::WRL::ComPtr;

    // Create the vertex buffer.
    {
        m_vertexBuffer = vertexBuffer->GetBuffer();

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        m_vertexBufferView.SizeInBytes = vertexBuffer->GetBufferWidth();
    }

    return true;
}

bool rendering::DXDevice::Present(std::string& errorMessage)
{    
    THROW_ERROR(m_swapChain->Present(1, 0),
        "Can't present Swap Chain!")

    return true;
}

#undef THROW_ERROR


void rendering::DXDevice::InitProperties(interpreter::NativeObject& nativeObject)
{
	using namespace interpreter;


#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

    Value& updateCurrentFrameIndex = GetOrCreateProperty(nativeObject, "updateCurrentFrameIndex");
    updateCurrentFrameIndex = CreateNativeMethod(nativeObject, 0, [](Value scope) {
        Value self = scope.GetProperty("self");
        DXDevice& device = static_cast<DXDevice&>(*NativeObject::ExtractNativeObject(self));

        device.UpdateCurrentFrameIndex();
        return Value();
    });

    Value& create = GetOrCreateProperty(nativeObject, "create");
    create = CreateNativeMethod(nativeObject, 2, [](Value scope) {
        Value self = scope.GetProperty("self");
        DXDevice& device = static_cast<DXDevice&>(*NativeObject::ExtractNativeObject(self));

        std::string error;
        bool res = device.Create(error);

        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

	Value& initProp = GetOrCreateProperty(nativeObject, "init");
	initProp = CreateNativeMethod(nativeObject, 2, [](Value scope) {
		Value self = scope.GetProperty("self");
		DXDevice& device = static_cast<DXDevice&>(*NativeObject::ExtractNativeObject(self));

		Value windowValue = scope.GetProperty("param0");
        Window* window = dynamic_cast<Window*>(NativeObject::ExtractNativeObject(windowValue));

        device.m_width = window->m_width;
		device.m_height = window->m_height;

        device.m_frameIndex = 0;
        device.m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(device.m_width), static_cast<float>(device.m_height));
        device.m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(device.m_width), static_cast<LONG>(device.m_height));
        device.m_rtvDescriptorSize = 0;

        Value commandQueueValue = scope.GetProperty("param1");
        DXCommandQueue* commandQueue = dynamic_cast<DXCommandQueue*>(NativeObject::ExtractNativeObject(commandQueueValue));

        if (!commandQueue) {
            THROW_EXCEPTION("Please supply a Command Queue!");
        }

        std::string errorMessage;
        {
            bool res = device.LoadPipeline(window->m_hwnd, commandQueue->GetCommandQueue(), errorMessage);
            if (!res) {
                THROW_EXCEPTION(errorMessage)
            }
        }

		return Value();
	});

    Value& load = GetOrCreateProperty(nativeObject, "load");
    load = CreateNativeMethod(nativeObject, 1, [](Value scope) {
        Value self = scope.GetProperty("self");
        DXDevice& device = static_cast<DXDevice&>(*NativeObject::ExtractNativeObject(self));

        Value vertexBufferValue = scope.GetProperty("param0");
        DXBuffer* vertexBuffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(vertexBufferValue));
        if (!vertexBuffer) {
            THROW_EXCEPTION("Please supply a Vertex Buffer!")
        }

        std::string errorMessage;
        {
            bool res = device.LoadAssets(vertexBuffer, errorMessage);
            if (!res) {
                THROW_EXCEPTION(errorMessage)
            }
        }

        return Value();
    });

    Value& present = GetOrCreateProperty(nativeObject, "present");
    present = CreateNativeMethod(nativeObject, 0, [](Value scope) {
        Value self = scope.GetProperty("self");
        NativeObject* selfContainer = static_cast<NativeObject*>(self.GetManagedValue());
        DXDevice& device = static_cast<DXDevice&>(selfContainer->GetNativeObject());

        std::string errorMessage;
        bool res = device.Present(errorMessage);

        if (!res) {
            THROW_EXCEPTION(errorMessage)
        }

        return Value();
    });

#undef THROW_EXCEPTION
}

ID3D12Device& rendering::DXDevice::GetDevice()
{
    return *m_device.Get();
}


CD3DX12_CPU_DESCRIPTOR_HANDLE rendering::DXDevice::GetCurrentRTVDescriptor() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
}

const D3D12_VERTEX_BUFFER_VIEW* rendering::DXDevice::GetVertexBufferView() const
{
    return &m_vertexBufferView;
}

ID3D12Resource* rendering::DXDevice::GetCurrentRenderTarget() const
{
    return m_renderTargets[m_frameIndex].Get();
}
