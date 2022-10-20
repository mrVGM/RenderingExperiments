#include "CL/dxWorlyTextureComputeCL.h"

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

namespace
{
    struct Settings
    {
        int m_cells1;
        int m_cells2;
        float m_blend;
    };
    struct ConstantBuffer
    {
        int m_texSize;
        Settings cells[3];
    };

    struct SRVBuffElement
    {
        float m_x;
        float m_y;
        float m_z;
    };
}

void rendering::DXWorlyTextureComputeCL::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

    Value& create = GetOrCreateProperty(nativeObject, "create");
    create = CreateNativeMethod(nativeObject, 2, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXWorlyTextureComputeCL* self = static_cast<DXWorlyTextureComputeCL*>(NativeObject::ExtractNativeObject(selfValue));

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
    populate = CreateNativeMethod(nativeObject, 6, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXWorlyTextureComputeCL* self = static_cast<DXWorlyTextureComputeCL*>(NativeObject::ExtractNativeObject(selfValue));

        Value constantBufferValue = scope.GetProperty("param0");
        DXBuffer* constantBuffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(constantBufferValue));

        if (!constantBuffer) {
            THROW_EXCEPTION("Please supply a constant buffer!")
        }

        Value texValue = scope.GetProperty("param1");
        DXTexture* tex = dynamic_cast<DXTexture*>(NativeObject::ExtractNativeObject(texValue));

        if (!tex) {
            THROW_EXCEPTION("Please supply a texture!")
        }

        Value srvUavHeapValue = scope.GetProperty("param2");
        DXDescriptorHeap* srvUavHeap = dynamic_cast<DXDescriptorHeap*>(NativeObject::ExtractNativeObject(srvUavHeapValue));

        if (!srvUavHeap) {
            THROW_EXCEPTION("Please supply a SRV|UAV heap!")
        }

        Value xValue = scope.GetProperty("param3");
        if (xValue.GetType() != ScriptingValueType::Number) {
            THROW_EXCEPTION("Please supply threadGroupCountX!")
        }

        Value yValue = scope.GetProperty("param4");
        if (yValue.GetType() != ScriptingValueType::Number) {
            THROW_EXCEPTION("Please supply threadGroupCountY!")
        }

        Value zValue = scope.GetProperty("param5");
        if (zValue.GetType() != ScriptingValueType::Number) {
            THROW_EXCEPTION("Please supply threadGroupCountZ!")
        }


        std::string error;
        bool res = self->Populate(
            constantBuffer->GetBuffer(),
            tex->GetTexture(),
            srvUavHeap->GetHeap(),
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
        DXWorlyTextureComputeCL* commandList = static_cast<DXWorlyTextureComputeCL*>(NativeObject::ExtractNativeObject(selfValue));

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
        DXWorlyTextureComputeCL* commandList = static_cast<DXWorlyTextureComputeCL*>(NativeObject::ExtractNativeObject(selfValue));

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

    Value& setupCB = GetOrCreateProperty(nativeObject, "setupCB");
    setupCB = CreateNativeMethod(nativeObject, 2, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXWorlyTextureComputeCL* self = static_cast<DXWorlyTextureComputeCL*>(NativeObject::ExtractNativeObject(selfValue));

        Value bufferValue = scope.GetProperty("param0");
        DXBuffer* buffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(bufferValue));

        if (!buffer) {
            THROW_EXCEPTION("Please supply a Constant Buffer!")
        }

        Value texSizeValue = scope.GetProperty("param1");
        if (texSizeValue.GetType() != ScriptingValueType::Number) {
            THROW_EXCEPTION("Please supply a Texture Size!")
        }
        int texSize = static_cast<int>(texSizeValue.GetNum());
        if (texSize <= 0) {
            THROW_EXCEPTION("Please supply a Valid Texture Size!")
        }
        std::string error;
        bool res = self->SetConstantBuffer(buffer->GetBuffer(), texSize, error);

        if (!res) {
            THROW_EXCEPTION(error)
        }
        return Value();
        });

    Value& setupSRVBuff = GetOrCreateProperty(nativeObject, "setupSRVBuff");
    setupSRVBuff = CreateNativeMethod(nativeObject, 1, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXWorlyTextureComputeCL* self = static_cast<DXWorlyTextureComputeCL*>(NativeObject::ExtractNativeObject(selfValue));

        Value buffersValue = scope.GetProperty("param0");
        std::list<Value> tmp;
        buffersValue.ToList(tmp);

        std::list<ID3D12Resource*> buffers;

        for (std::list<Value>::const_iterator it = tmp.begin(); it != tmp.end(); ++it) {
            DXBuffer* buffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(*it));

            if (!buffer) {
                THROW_EXCEPTION("Please supply a SRV Buffer!")
            }
            buffers.push_back(buffer->GetBuffer());
        }

        std::string error;
        bool res = self->SetSRVBuffer(buffers, error);

        if (!res) {
            THROW_EXCEPTION(error)
        }
        return Value();
    });

    Value& getSRVBufferSize = GetOrCreateProperty(nativeObject, "getSRVBufferSize");
    getSRVBufferSize = CreateNativeMethod(nativeObject, 0, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXWorlyTextureComputeCL* self = static_cast<DXWorlyTextureComputeCL*>(NativeObject::ExtractNativeObject(selfValue));

        std::list<int> sizes;
        self->GetSRVBufferSize(sizes);

        std::list<Value> sizeValues;
        for (std::list<int>::const_iterator it = sizes.begin(); it != sizes.end(); ++it) {
            sizeValues.push_back(Value(*it));
        }

        return Value::FromList(sizeValues);
    });

    Value& getSRVBufferStride = GetOrCreateProperty(nativeObject, "getSRVBufferStride");
    getSRVBufferStride = CreateNativeMethod(nativeObject, 0, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXWorlyTextureComputeCL* self = static_cast<DXWorlyTextureComputeCL*>(NativeObject::ExtractNativeObject(selfValue));

        return Value(self->GetSRVBufferStride());
    });

#undef THROW_EXCEPTION
}

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

bool rendering::DXWorlyTextureComputeCL::Create(
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
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

        CD3DX12_ROOT_PARAMETER1 rootParameters[ComputeRootParametersCount];
        rootParameters[ComputeRootCBV].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[ComputeRootUAVTable].InitAsDescriptorTable(1, ranges, D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[ComputeRootSRVTable].InitAsDescriptorTable(1, ranges + 1, D3D12_SHADER_VISIBILITY_ALL);

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

bool rendering::DXWorlyTextureComputeCL::Populate(
    ID3D12Resource* constantBuff,
    ID3D12Resource* tex,
    ID3D12DescriptorHeap* srvUavHeap,
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

    ID3D12DescriptorHeap* ppHeaps[] = { srvUavHeap };
    m_computeCL->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    CD3DX12_GPU_DESCRIPTOR_HANDLE uavHandle(srvUavHeap->GetGPUDescriptorHandleForHeapStart());
    CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(uavHandle, 1, m_srvUavDescriptorSize);

    m_computeCL->SetComputeRootConstantBufferView(ComputeRootCBV,constantBuff->GetGPUVirtualAddress());
    m_computeCL->SetComputeRootDescriptorTable(ComputeRootUAVTable, uavHandle);
    m_computeCL->SetComputeRootDescriptorTable(ComputeRootSRVTable, srvHandle);

    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(tex, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        m_computeCL->ResourceBarrier(1, &barrier);
    }

    m_computeCL->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);

    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(tex, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PRESENT);
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
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(tex, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_prepareForPixelCL->ResourceBarrier(1, &barrier);
    }

    THROW_ERROR(
        m_prepareForPixelCL->Close(),
        "Can't close Prepare for Pixel Command List!")
#pragma endregion


    return true;
}

bool rendering::DXWorlyTextureComputeCL::ExecuteCompute(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, int signal, std::string& error)
{
    ID3D12CommandList* ppCommandLists[] = { m_computeCL.Get() };
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    commandQueue->Signal(fence, signal);

    return true;
}

bool rendering::DXWorlyTextureComputeCL::ExecutePrepareForPS(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, int signal, std::string& error)
{
    ID3D12CommandList* ppCommandLists[] = { m_prepareForPixelCL.Get() };
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    commandQueue->Signal(fence, signal);

    return true;
}

bool rendering::DXWorlyTextureComputeCL::SetConstantBuffer(ID3D12Resource* buffer, int texSize, std::string& errorMessage)
{
    CD3DX12_RANGE readRange(0, 0);

    void* dst = nullptr;

    THROW_ERROR(
        buffer->Map(0, &readRange, &dst),
        "Can't map Constant Buffer!")

        ConstantBuffer cb;
    cb.m_texSize = texSize;

    Settings* currSettings = cb.cells;
    for (std::list<NoiseSettings>::const_iterator it = m_noiseSettings.begin(); it != m_noiseSettings.end(); ++it) {
        currSettings->m_cells1 = (*it).m_cells1;
        currSettings->m_cells2 = (*it).m_cells2;
        currSettings->m_blend = (*it).m_blend;

        ++currSettings;
    }

    memcpy(dst, &cb, sizeof(ConstantBuffer));
    buffer->Unmap(0, nullptr);

    return true;
}

bool rendering::DXWorlyTextureComputeCL::SetSRVBuffer(std::list<ID3D12Resource*> buffers, std::string& errorMessage)
{
    std::list<NoiseSettings>::const_iterator noiseSettingsIt = m_noiseSettings.begin();
    for (std::list<ID3D12Resource*>::iterator it = buffers.begin(); it != buffers.end(); ++it) {
        NoiseSettings currSettings = *noiseSettingsIt;
        ++noiseSettingsIt;

        ID3D12Resource* buffer = *it;

        CD3DX12_RANGE readRange(0, 0);

        void* dst = nullptr;

        THROW_ERROR(
            buffer->Map(0, &readRange, &dst),
            "Can't map SRV Buffer!")

        int srvBuffSize = currSettings.m_cells1;
        int arrSize = srvBuffSize * srvBuffSize * srvBuffSize;
        SRVBuffElement* elements = new SRVBuffElement[arrSize];

        for (int k = 0; k < srvBuffSize; ++k) {
            for (int i = 0; i < srvBuffSize; ++i) {
                for (int j = 0; j < srvBuffSize; ++j) {
                    int index = k * srvBuffSize * srvBuffSize + i * srvBuffSize + j;
                    SRVBuffElement& cur = elements[index];

                    float randX = (float)rand() / RAND_MAX;
                    float randY = (float)rand() / RAND_MAX;
                    float randZ = (float)rand() / RAND_MAX;

                    cur.m_x = j + randX;
                    cur.m_y = i + randY;
                    cur.m_z = k + randZ;
                }
            }
        }

        memcpy(dst, elements, arrSize * sizeof(SRVBuffElement));
        buffer->Unmap(0, nullptr);

        delete[] elements;
    }

    return true;
}

void rendering::DXWorlyTextureComputeCL::GetSRVBufferSize(std::list<int>& sizes) const
{
    for (std::list<NoiseSettings>::const_iterator it = m_noiseSettings.begin(); it != m_noiseSettings.end(); ++it) {
        const NoiseSettings& cur = *it;

        int s = cur.m_cells1 + cur.m_cells2;
        sizes.push_back(s * s * s * GetSRVBufferStride());
    }
}

int rendering::DXWorlyTextureComputeCL::GetSRVBufferStride() const
{
    return sizeof(SRVBuffElement);
}

rendering::DXWorlyTextureComputeCL::DXWorlyTextureComputeCL()
{
    NoiseSettings tmp;
    tmp.m_cells1 = 5;
    tmp.m_cells2 = 13;
    tmp.m_blend = 0.75;
    m_noiseSettings.push_back(tmp);

    tmp.m_cells1 = 10;
    tmp.m_cells2 = 13;
    tmp.m_blend = 0.75;
    m_noiseSettings.push_back(tmp);

    m_noiseSettings.push_back(tmp);
}

#undef THROW_ERROR