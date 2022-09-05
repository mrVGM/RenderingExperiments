#include "dxRenderCanvasesCL.h"

#include "nativeFunc.h"
#include "dxDevice.h"
#include "dxVertexShader.h"
#include "dxPixelShader.h"
#include "dxSwapChain.h"
#include "dxBuffer.h"
#include "dxDescriptorHeap.h"
#include "dxCanvasCL.h"

void rendering::DXRenderCanvasesCL::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

    Value& create = GetOrCreateProperty(nativeObject, "create");
    create = CreateNativeMethod(nativeObject, 1, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXRenderCanvasesCL* commandList = dynamic_cast<DXRenderCanvasesCL*>(NativeObject::ExtractNativeObject(selfValue));

        Value deviceValue = scope.GetProperty("param0");
        DXDevice* device = dynamic_cast<DXDevice*>(NativeObject::ExtractNativeObject(deviceValue));
        if (!device) {
            THROW_EXCEPTION("Please supply a device!")
        }

        std::string error;
        bool res = commandList->Create(&device->GetDevice(), error);
        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

    Value& execute = GetOrCreateProperty(nativeObject, "execute");
    execute = CreateNativeMethod(nativeObject, 4, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXRenderCanvasesCL* commandList = dynamic_cast<DXRenderCanvasesCL*>(NativeObject::ExtractNativeObject(selfValue));

        Value commandQueueValue = scope.GetProperty("param0");
        DXCommandQueue* commandQueue = dynamic_cast<DXCommandQueue*>(NativeObject::ExtractNativeObject(commandQueueValue));
        if (!commandQueue) {
            THROW_EXCEPTION("Please supply a Command Queue!")
        }
        
        Value fenceValue = scope.GetProperty("param1");
        DXFence* fence = dynamic_cast<DXFence*>(NativeObject::ExtractNativeObject(fenceValue));
        if (!fence) {
            THROW_EXCEPTION("Please supply a Fence!")
        }

        Value signalValue = scope.GetProperty("param2");
        if (signalValue.GetType() != ScriptingValueType::Number) {
            THROW_EXCEPTION("Please supply a Signal Value!")
        }
        int signal = static_cast<int>(signalValue.GetNum());

        Value canvasesListValue = scope.GetProperty("param3");
        std::vector<Value> canvasesList;
        canvasesListValue.ToList(canvasesList);
        if (canvasesList.size() == 0) {
            THROW_EXCEPTION("Please supply canvas CLs!")
        }

        std::vector<ID3D12GraphicsCommandList*> canvases;
        for (int i = 0; i < canvasesList.size(); ++i) {
            DXCanvasCL* canvas = dynamic_cast<DXCanvasCL*>(NativeObject::ExtractNativeObject(canvasesList[i]));
            if (!canvas) {
                THROW_EXCEPTION("Only Canvas CLs, please!")
            }
            canvases.push_back(canvas->GetCommandList());
        }

        std::string error;
        bool res = commandList->Execute(
            commandQueue->GetCommandQueue(),
            fence->GetFence(),
            signal,
            canvases,
            error);
        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

    Value& populate = GetOrCreateProperty(nativeObject, "populate");
    populate = CreateNativeMethod(nativeObject, 1, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXRenderCanvasesCL* commandList = dynamic_cast<DXRenderCanvasesCL*>(NativeObject::ExtractNativeObject(selfValue));

        Value swapChainValue = scope.GetProperty("param0");
        DXSwapChain* swapChain = dynamic_cast<DXSwapChain*>(NativeObject::ExtractNativeObject(swapChainValue));

        if (!swapChain) {
            THROW_EXCEPTION("Please supply a swap chain!")
        }

        std::string error;
        bool res = commandList->Populate(
            swapChain->GetCurrentRTVDescriptor(),
            swapChain->GetCurrentRenderTarget(),
            error);

        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

#undef THROW_EXCEPTION
}

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

bool rendering::DXRenderCanvasesCL::Create(ID3D12Device* device, std::string& errorMessage)
{
    using Microsoft::WRL::ComPtr;

    THROW_ERROR(
        device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)),
        "Can't create Command Allocator!")

    THROW_ERROR(
        device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_clearRTCL)),
        "Can't create Clear Command List!")

    THROW_ERROR(
        m_clearRTCL->Close(),
        "Can't close Clear Command List!")

    THROW_ERROR(
        device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_finishCL)),
        "Can't create Finish Command List!")

    THROW_ERROR(
        m_finishCL->Close(),
        "Can't close Finish Command List!")

    return true;
}

bool rendering::DXRenderCanvasesCL::Populate(CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle, ID3D12Resource* renderTarget, std::string& errorMessage)
{
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    THROW_ERROR(
        m_commandAllocator->Reset(),
        "Can't reset Command Allocator!")

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    THROW_ERROR(
        m_clearRTCL->Reset(m_commandAllocator.Get(), nullptr),
        "Can't reset Clear Command List!")

    // Indicate that the back buffer will be used as a render target.
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_clearRTCL->ResourceBarrier(1, &barrier);
    }

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_clearRTCL->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    THROW_ERROR(
        m_clearRTCL->Close(),
        "Can't close Clear Command List!")

    THROW_ERROR(
        m_finishCL->Reset(m_commandAllocator.Get(), nullptr),
        "Can't reset Finish Command List!")

    // Indicate that the back buffer will now be used to present.
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        m_finishCL->ResourceBarrier(1, &barrier);
    }

    THROW_ERROR(
        m_finishCL->Close(),
        "Can't close Finish Command List!")

    return true;
}

bool rendering::DXRenderCanvasesCL::Execute(
    ID3D12CommandQueue* commandQueue,
    ID3D12Fence* fence,
    int signal,
    const std::vector<ID3D12GraphicsCommandList*>& canvases,
    std::string& errorMessage)
{
    
    ID3D12CommandList* clearCommandList[] = { m_clearRTCL.Get() };
    commandQueue->ExecuteCommandLists(_countof(clearCommandList), clearCommandList);

    ID3D12CommandList** canvasCommandLists = new ID3D12CommandList*[canvases.size()];
    for (int i = 0; i < canvases.size(); ++i) {
        canvasCommandLists[i] = canvases[i];
    }
    commandQueue->ExecuteCommandLists(canvases.size(), canvasCommandLists);
    delete[] canvasCommandLists;

    ID3D12CommandList* finishCommandList[] = { m_finishCL.Get() };
    commandQueue->ExecuteCommandLists(_countof(finishCommandList), finishCommandList);

    THROW_ERROR(
        commandQueue->Signal(fence, signal),
        "Can't signal fence!")

    return true;
}

#undef THROW_ERROR