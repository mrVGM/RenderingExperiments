#include "CL/dxGeometryPassStartCL.h"

#include "nativeFunc.h"

#include "dxDevice.h"
#include "dxDescriptorHeap.h"
#include "dxCommandQueue.h"
#include "dxFence.h"
#include "dxTexture.h"

void rendering::DXGeometryPassStartCL::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

    Value& create = GetOrCreateProperty(nativeObject, "create");
    create = CreateNativeMethod(nativeObject, 2, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXGeometryPassStartCL* self = static_cast<DXGeometryPassStartCL*>(NativeObject::ExtractNativeObject(selfValue));

        Value deviceValue = scope.GetProperty("param0");
        DXDevice* device = dynamic_cast<DXDevice*>(NativeObject::ExtractNativeObject(deviceValue));

        if (!device) {
            THROW_EXCEPTION("Please supply a device!")
        }

        Value diffuseTexValue = scope.GetProperty("param1");
        DXTexture* diffuseTex = dynamic_cast<DXTexture*>(NativeObject::ExtractNativeObject(diffuseTexValue));

        if (!diffuseTex) {
            THROW_EXCEPTION("Please supply a diffuse texture!")
        }

        std::string error;
        bool res = self->Create(
            &device->GetDevice(),
            diffuseTex->GetTexture(),
            error
        );

        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

    Value& execute = GetOrCreateProperty(nativeObject, "execute");
    execute = CreateNativeMethod(nativeObject, 3, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXGeometryPassStartCL* self = static_cast<DXGeometryPassStartCL*>(NativeObject::ExtractNativeObject(selfValue));

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

        std::string error;
        bool res = self->Execute(commandQueue->GetCommandQueue(), fence->GetFence(), signal, error);
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

bool rendering::DXGeometryPassStartCL::Create(
    ID3D12Device* device,
    ID3D12Resource* diffuseTexture,
    std::string& errorMessage)
{
    using Microsoft::WRL::ComPtr;

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = 4;
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

        device->CreateRenderTargetView(diffuseTexture, nullptr, rtvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);
    }

    THROW_ERROR(
        device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)),
        "Can't create Command Allocator!")

    THROW_ERROR(
        device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)),
        "Can't create Command List!")

    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::CD3DX12_RESOURCE_BARRIER::Transition(diffuseTexture, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_commandList->ResourceBarrier(1, &barrier);
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);


    THROW_ERROR(
        m_commandList->Close(),
        "Can't close command List!")

    return true;
}

bool rendering::DXGeometryPassStartCL::Execute(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, int signal, std::string& error)
{
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    commandQueue->Signal(fence, signal);

    return true;
}

ID3D12DescriptorHeap* rendering::DXGeometryPassStartCL::GetRTVHeap() const
{
    return m_rtvHeap.Get();
}

#undef THROW_ERROR