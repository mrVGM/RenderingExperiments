#include "dxCopyCL.h"

#include "nativeFunc.h"
#include "dxDevice.h"
#include "dxVertexShader.h"
#include "dxPixelShader.h"
#include "dxSwapChain.h"
#include "dxBuffer.h"
#include "dxDescriptorHeap.h"
#include "dxCanvasCL.h"
#include "IDXResource.h"

void rendering::DXCopyCL::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

    Value& create = GetOrCreateProperty(nativeObject, "create");
    create = CreateNativeMethod(nativeObject, 1, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXCopyCL* commandList = dynamic_cast<DXCopyCL*>(NativeObject::ExtractNativeObject(selfValue));

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
    execute = CreateNativeMethod(nativeObject, 3, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXCopyCL* commandList = dynamic_cast<DXCopyCL*>(NativeObject::ExtractNativeObject(selfValue));

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
        bool res = commandList->Execute(
            commandQueue->GetCommandQueue(),
            fence->GetFence(),
            signal,
            error);

        if (!res) {
            THROW_EXCEPTION(error)
        }
        return Value();
    });

    Value& populate = GetOrCreateProperty(nativeObject, "populate");
    populate = CreateNativeMethod(nativeObject, 2, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXCopyCL* commandList = dynamic_cast<DXCopyCL*>(NativeObject::ExtractNativeObject(selfValue));

        Value dstValue = scope.GetProperty("param0");
        IDXResource* dstRes = dynamic_cast<IDXResource*>(NativeObject::ExtractNativeObject(dstValue));

        if (!dstRes) {
            THROW_EXCEPTION("Please supply a dst resource!")
        }

        Value srcValue = scope.GetProperty("param1");
        IDXResource* srcRes = dynamic_cast<IDXResource*>(NativeObject::ExtractNativeObject(srcValue));

        if (!srcRes) {
            THROW_EXCEPTION("Please supply a src resource!")
        }

        std::string error;
        bool res = commandList->Populate(
            dstRes->GetResource(),
            srcRes->GetResource(),
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

bool rendering::DXCopyCL::Create(ID3D12Device* device, std::string& errorMessage)
{
    using Microsoft::WRL::ComPtr;

    THROW_ERROR(
        device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&m_commandAllocator)),
        "Can't create Command Allocator!")

    THROW_ERROR(
        device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)),
        "Can't create Command List!")

    THROW_ERROR(
        m_commandList->Close(),
        "Can't close Command List!")
    
    return true;
}

bool rendering::DXCopyCL::Populate(ID3D12Resource* dst, ID3D12Resource* src, std::string& errorMessage)
{
    THROW_ERROR(
        m_commandAllocator->Reset(),
        "Can't reset Command Allocator!")

    THROW_ERROR(
        m_commandList->Reset(m_commandAllocator.Get(), nullptr),
        "Can't reset Clear Command List!")

    m_commandList->CopyResource(dst, src);


    THROW_ERROR(
        m_commandList->Close(),
        "Can't close Finish Command List!")

    return true;
}

bool rendering::DXCopyCL::Execute(
    ID3D12CommandQueue* commandQueue,
    ID3D12Fence* fence,
    int signal,
    std::string& errorMessage)
{
    ID3D12CommandList* clearCommandList[] = { m_commandList.Get() };
    commandQueue->ExecuteCommandLists(_countof(clearCommandList), clearCommandList);

    THROW_ERROR(
        commandQueue->Signal(fence, signal),
        "Can't signal fence!")

    return true;
}

#undef THROW_ERROR