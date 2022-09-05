#include "dxUpdateTextureCL.h"

#include "nativeFunc.h"
#include "dxDevice.h"
#include "dxVertexShader.h"
#include "dxPixelShader.h"
#include "dxSwapChain.h"
#include "dxBuffer.h"
#include "dxDescriptorHeap.h"
#include "dxCanvasCL.h"
#include "IDXResource.h"

void rendering::DXUpdateTextureCL::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

    Value& create = GetOrCreateProperty(nativeObject, "create");
    create = CreateNativeMethod(nativeObject, 1, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXUpdateTextureCL* commandList = dynamic_cast<DXUpdateTextureCL*>(NativeObject::ExtractNativeObject(selfValue));

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
        DXUpdateTextureCL* commandList = dynamic_cast<DXUpdateTextureCL*>(NativeObject::ExtractNativeObject(selfValue));

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
        DXUpdateTextureCL* commandList = dynamic_cast<DXUpdateTextureCL*>(NativeObject::ExtractNativeObject(selfValue));

        Value deviceValue = scope.GetProperty("param0");
        DXDevice* device = dynamic_cast<DXDevice*>(NativeObject::ExtractNativeObject(deviceValue));
        if (!device) {
            THROW_EXCEPTION("Please supply a device!")
        }

        Value texValue = scope.GetProperty("param1");
        DXTexture* tex = dynamic_cast<DXTexture*>(NativeObject::ExtractNativeObject(texValue));
        if (!tex) {
            THROW_EXCEPTION("Please supply a texture!")
        }

        std::string error;
        bool res = commandList->Populate(
            &device->GetDevice(),
            tex,
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

bool rendering::DXUpdateTextureCL::Create(ID3D12Device* device, std::string& errorMessage)
{
    using Microsoft::WRL::ComPtr;

    THROW_ERROR(
        device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)),
        "Can't create Command Allocator!")

    THROW_ERROR(
        device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)),
        "Can't create Command List!")

    THROW_ERROR(
        m_commandList->Close(),
        "Can't close Command List!")
    
    return true;
}

bool rendering::DXUpdateTextureCL::Populate(ID3D12Device* device, DXTexture* tex, std::string& errorMessage)
{
    THROW_ERROR(
        m_commandAllocator->Reset(),
        "Can't reset Command Allocator!")

    THROW_ERROR(
        m_commandList->Reset(m_commandAllocator.Get(), nullptr),
        "Can't reset Command List!")

    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(tex->GetTexture(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_commandList->ResourceBarrier(1, &barrier);
    }

    THROW_ERROR(
        m_commandList->Close(),
        "Can't close  Command List!")

    return true;
}

bool rendering::DXUpdateTextureCL::Execute(
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

void rendering::DXUpdateTextureCL::GenerateTexData(std::vector<float>& data, DXTexture* tex)
{
    const UINT rowPitch = tex->GetTextureWidth() * tex->GetTexturePixelSize();
    const UINT textureSize = rowPitch * tex->GetTextureHeight();

    data = std::vector<float>(textureSize / 4);
    float* pData = &data[0];

    for (UINT n = 0; n < textureSize / 4; n += tex->GetTexturePixelSize() / 4)
    {
        pData[n] = 1;        // R
        pData[n + 1] = 1;    // G
        pData[n + 2] = 1;    // B
        pData[n + 3] = 1;    // A
    }
}

#undef THROW_ERROR