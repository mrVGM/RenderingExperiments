#include "compute/dxNoiseTexture.h"

#include "nativeFunc.h"

#include "dxDevice.h"
#include "dxDescriptorHeap.h"
#include "dxComputeShader.h"
#include "dxCommandQueue.h"
#include "dxComputeCommandQueue.h"
#include "dxBuffer.h"
#include "dxTexture.h"
#include "dxFence.h"

#include <stdlib.h>

void rendering::compute::DXNoiseTexture::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

    Value& p_constantBuffer = GetOrCreateProperty(nativeObject, "constantBuffer");
    Value& p_dataBuffer = GetOrCreateProperty(nativeObject, "dataBuffer");
    Value& p_texture = GetOrCreateProperty(nativeObject, "texture");
    Value& p_descriptorHeap = GetOrCreateProperty(nativeObject, "descriptorHeap");

    Value& setConstantBuffer = GetOrCreateProperty(nativeObject, "setConstantBuffer");
    setConstantBuffer = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXNoiseTexture* self = static_cast<DXNoiseTexture*>(NativeObject::ExtractNativeObject(selfValue));

        Value constantBufferValue = scope.GetProperty("param0");
        DXBuffer* constantBuffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(constantBufferValue));

        if (!constantBuffer) {
            THROW_EXCEPTION("Please supply constant buffer!")
        }

        p_constantBuffer = constantBufferValue;
        self->m_constantBuffer = constantBuffer->GetBuffer();

        return Value();
    });

    Value& setDataBuffer = GetOrCreateProperty(nativeObject, "setDataBuffer");
    setDataBuffer = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXNoiseTexture* self = static_cast<DXNoiseTexture*>(NativeObject::ExtractNativeObject(selfValue));

        Value dataBufferValue = scope.GetProperty("param0");
        DXBuffer* dataBuffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(dataBufferValue));

        if (!dataBuffer) {
            THROW_EXCEPTION("Please supply data buffer!")
        }

        p_dataBuffer = dataBufferValue;
        self->m_dataBuffer = dataBuffer->GetBuffer();

        return Value();
    });

    Value& setTexture = GetOrCreateProperty(nativeObject, "setTexture");
    setTexture = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXNoiseTexture* self = static_cast<DXNoiseTexture*>(NativeObject::ExtractNativeObject(selfValue));

        Value textureValue = scope.GetProperty("param0");
        DXTexture* texture = dynamic_cast<DXTexture*>(NativeObject::ExtractNativeObject(textureValue));

        if (!texture) {
            THROW_EXCEPTION("Please supply texture!")
        }

        p_texture = textureValue;
        self->m_texture = texture->GetTexture();

        return Value();
    });

    Value& setDescriptorHeap = GetOrCreateProperty(nativeObject, "setDescriptorHeap");
    setDescriptorHeap = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXNoiseTexture* self = static_cast<DXNoiseTexture*>(NativeObject::ExtractNativeObject(selfValue));

        Value descriptorHeapValue = scope.GetProperty("param0");
        DXDescriptorHeap* descriptorHeap = dynamic_cast<DXDescriptorHeap*>(NativeObject::ExtractNativeObject(descriptorHeapValue));

        if (!descriptorHeap) {
            THROW_EXCEPTION("Please Descriptor Heap!")
        }

        p_descriptorHeap = descriptorHeapValue;
        self->m_descriptorHeap = descriptorHeap->GetHeap();

        return Value();
    });


    Value& create = GetOrCreateProperty(nativeObject, "create");
    create = CreateNativeMethod(nativeObject, 2, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXNoiseTexture* self = static_cast<DXNoiseTexture*>(NativeObject::ExtractNativeObject(selfValue));

        Value deviceValue = scope.GetProperty("param0");
        DXDevice* device = dynamic_cast<DXDevice*>(NativeObject::ExtractNativeObject(deviceValue));

        if (!device) {
            THROW_EXCEPTION("Please supply a device!")
        }

        Value computeShaderValue = scope.GetProperty("param1");
        DXComputeShader* computeShader = dynamic_cast<DXComputeShader*>(NativeObject::ExtractNativeObject(computeShaderValue));

        if (!computeShader) {
            THROW_EXCEPTION("Please supply a compute shader!")
        }

        std::string error;
        bool res = self->Create(
            &device->GetDevice(),
            computeShader->GetCompiledShader(),
            error);

        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

    Value& populate = GetOrCreateProperty(nativeObject, "populate");
    populate = CreateNativeMethod(nativeObject, 3, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXNoiseTexture* self = static_cast<DXNoiseTexture*>(NativeObject::ExtractNativeObject(selfValue));

        Value xValue = scope.GetProperty("param0");
        if (xValue.GetType() != ScriptingValueType::Number) {
            THROW_EXCEPTION("Please supply threadGroupCountX!")
        }

        Value yValue = scope.GetProperty("param1");
        if (yValue.GetType() != ScriptingValueType::Number) {
            THROW_EXCEPTION("Please supply threadGroupCountY!")
        }

        Value zValue = scope.GetProperty("param2");
        if (zValue.GetType() != ScriptingValueType::Number) {
            THROW_EXCEPTION("Please supply threadGroupCountZ!")
        }


        std::string error;
        bool res = self->Populate(
            static_cast<int>(xValue.GetNum()),
            static_cast<int>(yValue.GetNum()),
            static_cast<int>(zValue.GetNum()),
            error);

        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

    Value& executeCompute = GetOrCreateProperty(nativeObject, "executeCompute");
    executeCompute = CreateNativeMethod(nativeObject, 3, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXNoiseTexture* commandList = static_cast<DXNoiseTexture*>(NativeObject::ExtractNativeObject(selfValue));

        Value commandQueueValue = scope.GetProperty("param0");
        DXComputeCommandQueue* commandQueue = dynamic_cast<DXComputeCommandQueue*>(NativeObject::ExtractNativeObject(commandQueueValue));
        if (!commandQueue) {
            THROW_EXCEPTION("Please supply a Compute Command Queue!")
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
        bool res = commandList->ExecuteCompute(commandQueue->GetCommandQueue(), fence->GetFence(), signal, error);
        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

    Value& executePrepareForPS = GetOrCreateProperty(nativeObject, "executePrepareForPS");
    executePrepareForPS = CreateNativeMethod(nativeObject, 3, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXNoiseTexture* commandList = static_cast<DXNoiseTexture*>(NativeObject::ExtractNativeObject(selfValue));

        Value commandQueueValue = scope.GetProperty("param0");
        DXCommandQueue* commandQueue = dynamic_cast<DXCommandQueue*>(NativeObject::ExtractNativeObject(commandQueueValue));
        if (!commandQueue) {
            THROW_EXCEPTION("Please supply a Graphics Command Queue!")
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
        bool res = commandList->ExecutePrepareForPS(commandQueue->GetCommandQueue(), fence->GetFence(), signal, error);
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

bool rendering::compute::DXNoiseTexture::Create(
    ID3D12Device* device,
    ID3DBlob* computeShader,
    std::string& errorMessage)
{
    using Microsoft::WRL::ComPtr;
    m_srvUavDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Compute root signature.
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

        CD3DX12_ROOT_PARAMETER1 rootParameters[ComputeRootParametersCount];
        rootParameters[ComputeRootCBV].InitAsConstantBufferView(0, 0);
        rootParameters[ComputeRootUAVTable].InitAsDescriptorTable(1, ranges);
        rootParameters[ComputeRootSRVTable].InitAsDescriptorTable(1, ranges + 1);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC computeRootSignatureDesc;
        computeRootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        THROW_ERROR(
            D3DX12SerializeVersionedRootSignature(&computeRootSignatureDesc, featureData.HighestVersion, &signature, &error),
            "Can't serialize Compute Root Signature!")


        THROW_ERROR(
            device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)),
            "Can't create Compute Root Signature!")
    }


    // Describe and create the compute pipeline state object (PSO).
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
        computePsoDesc.pRootSignature = m_rootSignature.Get();
        computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(computeShader);

        THROW_ERROR(
            device->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_pipelineState)),
            "Can't create Compute Pipeline State!"
        )
    }

    THROW_ERROR(
        device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&m_computeAllocator)),
        "Can't create Compute Command Allocator!")

    THROW_ERROR(
        device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_computeAllocator.Get(), nullptr, IID_PPV_ARGS(&m_computeCL)),
        "Can't create Compute Command List!")

    THROW_ERROR(
        m_computeCL->Close(),
        "Can't close Compute command List!")

    THROW_ERROR(
        device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_prepareForPixelAllocator)),
        "Can't create Prepare for Pixel Command Allocator!")

    THROW_ERROR(
        device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_prepareForPixelAllocator.Get(), nullptr, IID_PPV_ARGS(&m_prepareForPixelCL)),
        "Can't create Prepare for Pixel Command List!")

    THROW_ERROR(
        m_prepareForPixelCL->Close(),
        "Can't close Prepare for Pixel command List!")

    return true;
}

bool rendering::compute::DXNoiseTexture::Populate(
    int threadGroupCountX,
    int threadGroupCountY,
    int threadGroupCountZ,
    std::string& errorMessage)
{
#pragma region Compute Command List
    THROW_ERROR(
        m_computeAllocator->Reset(),
        "Can't reset Compute Command Allocator!")

    THROW_ERROR(
        m_computeCL->Reset(m_computeAllocator.Get(), m_pipelineState.Get()),
        "Can't reset Command List!")

    m_computeCL->SetPipelineState(m_pipelineState.Get());
    m_computeCL->SetComputeRootSignature(m_rootSignature.Get());

    ID3D12DescriptorHeap* ppHeaps[] = { m_descriptorHeap };
    m_computeCL->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    CD3DX12_GPU_DESCRIPTOR_HANDLE uavHandle(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());
    CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(uavHandle, 1, m_srvUavDescriptorSize);

    m_computeCL->SetComputeRootConstantBufferView(ComputeRootCBV, m_constantBuffer->GetGPUVirtualAddress());
    m_computeCL->SetComputeRootDescriptorTable(ComputeRootUAVTable, uavHandle);
    m_computeCL->SetComputeRootDescriptorTable(ComputeRootSRVTable, srvHandle);

    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_texture, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        m_computeCL->ResourceBarrier(1, &barrier);
    }

    m_computeCL->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);

    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_texture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PRESENT);
        m_computeCL->ResourceBarrier(1, &barrier);
    }

    THROW_ERROR(
        m_computeCL->Close(),
        "Can't close Command List!")
#pragma endregion

#pragma region Prepare for Pixel Command List
    THROW_ERROR(
        m_prepareForPixelAllocator->Reset(),
        "Can't reset Prepare for Pixel Command Allocator!")

    THROW_ERROR(
        m_prepareForPixelCL->Reset(m_prepareForPixelAllocator.Get(), nullptr),
        "Can't reset Prepare for Pixel Command List!")

    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_texture, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_prepareForPixelCL->ResourceBarrier(1, &barrier);
    }

    THROW_ERROR(
        m_prepareForPixelCL->Close(),
        "Can't close Prepare for Pixel Command List!")
#pragma endregion


    return true;
}

bool rendering::compute::DXNoiseTexture::ExecuteCompute(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, int signal, std::string& error)
{
    ID3D12CommandList* ppCommandLists[] = { m_computeCL.Get() };
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    commandQueue->Signal(fence, signal);

    return true;
}

bool rendering::compute::DXNoiseTexture::ExecutePrepareForPS(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, int signal, std::string& error)
{
    ID3D12CommandList* ppCommandLists[] = { m_prepareForPixelCL.Get() };
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    commandQueue->Signal(fence, signal);

    return true;
}

#undef THROW_ERROR