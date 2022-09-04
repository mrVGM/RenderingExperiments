#include "dxComputeCL.h"

#include "nativeFunc.h"
#include "dxDevice.h"
#include "dxSwapChain.h"
#include "dxBuffer.h"
#include "dxDescriptorHeap.h"
#include "dxComputeShader.h"

void rendering::DXComputeCL::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

    Value& create = GetOrCreateProperty(nativeObject, "create");
    create = CreateNativeMethod(nativeObject, 2, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXComputeCL* commandList = dynamic_cast<DXComputeCL*>(NativeObject::ExtractNativeObject(selfValue));

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
        bool res = commandList->Create(
            &device->GetDevice(),
            computeShader->GetCompiledShader(),
            error);

        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

    Value& populate = GetOrCreateProperty(nativeObject, "populate");
    populate = CreateNativeMethod(nativeObject, 5, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXComputeCL* commandList = dynamic_cast<DXComputeCL*>(NativeObject::ExtractNativeObject(selfValue));

        Value srvUavHeapValue = scope.GetProperty("param1");
        DXDescriptorHeap* srvUavHeap = dynamic_cast<DXDescriptorHeap*>(NativeObject::ExtractNativeObject(srvUavHeapValue));

        if (!srvUavHeap) {
            THROW_EXCEPTION("Please supply a SRV|UAV heap!")
        }

        Value constantBufferValue = scope.GetProperty("param1");
        DXBuffer* constantBuffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(constantBufferValue));

        if (!constantBuffer) {
            THROW_EXCEPTION("Please supply a constant buffer!")
        }

        Value xValue = scope.GetProperty("param2");
        if (xValue.GetType() != ScriptingValueType::Number) {
            THROW_EXCEPTION("Please supply threadGroupCountX!")
        }

        Value yValue = scope.GetProperty("param3");
        if (yValue.GetType() != ScriptingValueType::Number) {
            THROW_EXCEPTION("Please supply threadGroupCountY!")
        }

        Value zValue = scope.GetProperty("param4");
        if (zValue.GetType() != ScriptingValueType::Number) {
            THROW_EXCEPTION("Please supply threadGroupCountZ!")
        }

        std::string error;
        bool res = commandList->Populate(
            srvUavHeap->GetHeap(),
            constantBuffer->GetBuffer(),
            static_cast<int>(xValue.GetNum()),
            static_cast<int>(yValue.GetNum()),
            static_cast<int>(zValue.GetNum()),
            error);

        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

    Value& execute = GetOrCreateProperty(nativeObject, "execute");
    execute = CreateNativeMethod(nativeObject, 3, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXComputeCL* commandList = dynamic_cast<DXComputeCL*>(NativeObject::ExtractNativeObject(selfValue));

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
        bool res = commandList->Execute(commandQueue->GetCommandQueue(), fence->GetFence(), signal, error);
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

bool rendering::DXComputeCL::Create(
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
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

        CD3DX12_ROOT_PARAMETER1 rootParameters[ComputeRootParametersCount];
        rootParameters[ComputeRootCBV].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[ComputeRootSRVTable].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[ComputeRootUAVTable].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);

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
        device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&m_commandAllocator)),
        "Can't create Compute Command Allocator!")

    THROW_ERROR(
        device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)),
        "Can't create Compute Command List!")

    THROW_ERROR(
        m_commandList->Close(),
        "Can't close Compute command List!")

    return true;
}

bool rendering::DXComputeCL::Populate(
    ID3D12DescriptorHeap* srvUavHeap,
    ID3D12Resource* constantBuff,
    int threadGroupCountX,
    int threadGroupCountY,
    int threadGroupCountZ,
    std::string& errorMessage)
{
    THROW_ERROR(
        m_commandAllocator->Reset(),
        "Can't reset Command Allocator!")

    THROW_ERROR(
        m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()),
        "Can't reset Command List!")

    m_commandList->SetPipelineState(m_pipelineState.Get());
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    ID3D12DescriptorHeap* ppHeaps[] = { srvUavHeap };
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(srvUavHeap->GetGPUDescriptorHandleForHeapStart());
    CD3DX12_GPU_DESCRIPTOR_HANDLE uavHandle(srvHandle, 1, m_srvUavDescriptorSize);

    m_commandList->SetComputeRootConstantBufferView(ComputeRootCBV,constantBuff->GetGPUVirtualAddress());
    m_commandList->SetComputeRootDescriptorTable(ComputeRootSRVTable, srvHandle);
    m_commandList->SetComputeRootDescriptorTable(ComputeRootUAVTable, uavHandle);

    m_commandList->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);

    THROW_ERROR(
        m_commandList->Close(),
        "Can't close Command List!")

    return true;
}

bool rendering::DXComputeCL::Execute(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, int signal, std::string& error)
{
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    commandQueue->Signal(fence, signal);

    return true;
}

ID3D12GraphicsCommandList* rendering::DXComputeCL::GetCommandList() const
{
    return m_commandList.Get();
}


#undef THROW_ERROR