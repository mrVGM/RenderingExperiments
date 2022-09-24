#include "deferred/dxDisplay3DCL.h"

#include "nativeFunc.h"

#include "dxDevice.h"
#include "dxVertexShader.h"
#include "dxPixelShader.h"
#include "dxSwapChain.h"
#include "dxBuffer.h"
#include "dxDescriptorHeap.h"
#include "dxCommandQueue.h"
#include "dxFence.h"
#include "deferred/gBuffer.h"
#include "dxTexture.h"

void rendering::deferred::DXLitMatCL::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

    Value& create = GetOrCreateProperty(nativeObject, "create");
    create = CreateNativeMethod(nativeObject, 4, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXLitMatCL* self = static_cast<DXLitMatCL*>(NativeObject::ExtractNativeObject(selfValue));

        Value deviceValue = scope.GetProperty("param0");
        DXDevice* device = dynamic_cast<DXDevice*>(NativeObject::ExtractNativeObject(deviceValue));

        if (!device) {
            THROW_EXCEPTION("Please supply a device!")
        }

        Value vertexShaderValue = scope.GetProperty("param1");
        DXVertexShader* vertexShader = dynamic_cast<DXVertexShader*>(NativeObject::ExtractNativeObject(vertexShaderValue));

        if (!vertexShader) {
            THROW_EXCEPTION("Please supply a vertex shader!")
        }

        Value pixelShaderValue = scope.GetProperty("param2");
        DXPixelShader* pixelShader = dynamic_cast<DXPixelShader*>(NativeObject::ExtractNativeObject(pixelShaderValue));

        if (!pixelShader) {
            THROW_EXCEPTION("Please supply a pixel shader!")
        }

        Value constantBufferValue = scope.GetProperty("param3");
        DXBuffer* constantBuffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(constantBufferValue));

        if (!constantBuffer) {
            THROW_EXCEPTION("Please supply a Constant Buffer!")
        }

        std::string error;
        bool res = self->Create(
            &device->GetDevice(),
            vertexShader->GetCompiledShader(),
            pixelShader->GetCompiledShader(),
            constantBuffer->GetBuffer(),
            error);

        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

    Value& populate = GetOrCreateProperty(nativeObject, "populate");
    populate = CreateNativeMethod(nativeObject, 4, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXLitMatCL* self = static_cast<DXLitMatCL*>(NativeObject::ExtractNativeObject(selfValue));

        Value vertexBufferValue = scope.GetProperty("param0");
        DXBuffer* vertexBuffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(vertexBufferValue));

        if (!vertexBuffer) {
            THROW_EXCEPTION("Please supply a vertex buffer!")
        }

        Value indexBufferValue = scope.GetProperty("param1");
        DXBuffer* indexBuffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(indexBufferValue));

        if (!indexBuffer) {
            THROW_EXCEPTION("Please supply an index buffer!")
        }

        Value instanceBufferValue = scope.GetProperty("param2");
        DXBuffer* instanceBuffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(instanceBufferValue));

        if (!instanceBuffer) {
            THROW_EXCEPTION("Please supply an instance buffer!")
        }

        Value gBufferValue = scope.GetProperty("param3");
        deferred::GBuffer* gBuffer = dynamic_cast<deferred::GBuffer*>(NativeObject::ExtractNativeObject(gBufferValue));
        if (!gBuffer) {
            THROW_EXCEPTION("Please supply a GBuffer!")
        }

        Value diffuseTexValue = gBufferValue.GetManagedValue()->GetProperty("diffuseTexture");
        DXTexture* diffuseTex = dynamic_cast<DXTexture*>(NativeObject::ExtractNativeObject(diffuseTexValue));

        Value w = gBufferValue.GetProperty("width");
        Value h = gBufferValue.GetProperty("height");

        CD3DX12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(w.GetNum()), static_cast<float>(h.GetNum()));
        CD3DX12_RECT scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(w.GetNum()), static_cast<LONG>(h.GetNum()));

        std::string error;
        bool res = self->Populate(
            &viewport,
            &scissorRect,
            gBuffer->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart(),
            diffuseTex->GetTexture(),
            vertexBuffer->GetBuffer(),
            vertexBuffer->GetBufferWidth(),
            vertexBuffer->GetStride(),
            indexBuffer->GetBuffer(),
            indexBuffer->GetBufferWidth(),
            instanceBuffer->GetBuffer(),
            instanceBuffer->GetBufferWidth(),
            instanceBuffer->GetStride(),
            error);

        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

    Value& execute = GetOrCreateProperty(nativeObject, "execute");
    execute = CreateNativeMethod(nativeObject, 3, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXLitMatCL* self = static_cast<DXLitMatCL*>(NativeObject::ExtractNativeObject(selfValue));

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

    Value& executeAsync = GetOrCreateProperty(nativeObject, "executeAsync");
    executeAsync = CreateNativeMethod(nativeObject, 1, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXLitMatCL* self = static_cast<DXLitMatCL*>(NativeObject::ExtractNativeObject(selfValue));

        Value commandQueueValue = scope.GetProperty("param0");
        DXCommandQueue* commandQueue = dynamic_cast<DXCommandQueue*>(NativeObject::ExtractNativeObject(commandQueueValue));
        if (!commandQueue) {
            THROW_EXCEPTION("Please supply a Command Queue!")
        }

        std::string error;
        bool res = self->ExecuteAsync(commandQueue->GetCommandQueue(), error);
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

bool rendering::deferred::DXLitMatCL::Create(
    ID3D12Device* device,
    ID3DBlob* vertexShader,
    ID3DBlob* pixelShader,
    ID3D12Resource* constantBuffer,
    std::string& errorMessage)
{
    using Microsoft::WRL::ComPtr;

    m_constantBuffer = constantBuffer;

    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        // Allow input layout and deny uneccessary access to certain pipeline stages.
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        CD3DX12_ROOT_PARAMETER1 rootParameters[1];
        rootParameters[0].InitAsConstantBufferView(0, 0);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(1, rootParameters, 0, nullptr, rootSignatureFlags);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        THROW_ERROR(
            D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error),
            "Can't serialize a root signature!")

        THROW_ERROR(
            device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)),
            "Can't create a root signature!")
    }


    // Create the pipeline state, which includes compiling and loading shaders.
    {
        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

            { "OBJECT_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 0 },
            { "OBJECT_ROTATION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 12, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 0 },
            { "OBJECT_SCALE", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 28, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 0 },
        };

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader);
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader);
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
        psoDesc.SampleDesc.Count = 1;
        THROW_ERROR(
            device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)),
            "Can't create Graphics Pipeline State!")
    }

    THROW_ERROR(
        device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)),
        "Can't create Command Allocator!")

    THROW_ERROR(
        device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)),
        "Can't create Command List!")

    THROW_ERROR(
        m_commandList->Close(),
        "Can't close command List!")

    return true;
}

bool rendering::deferred::DXLitMatCL::Populate(
    const CD3DX12_VIEWPORT* viewport,
    CD3DX12_RECT* scissorRect,
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle,
    ID3D12Resource* renderTarget,
    ID3D12Resource* vertexBuffer,
    int vertexBufferSize,
    int vertexBufferStride,
    ID3D12Resource* indexBuffer,
    int indexBufferSize,
    ID3D12Resource* instanceBuffer,
    int instanceBufferSize,
    int instanceBufferStride,
    std::string& errorMessage)
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
        m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()),
        "Can't reset Command List!")

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress());

#if false
    // Indicate that the back buffer will be used as a render target.
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_commandList->ResourceBarrier(1, &barrier);
    }
#endif

    m_commandList->RSSetViewports(1, viewport);
    m_commandList->RSSetScissorRects(1, scissorRect);

    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[2];
    D3D12_VERTEX_BUFFER_VIEW& realVertexBufferView = vertexBufferViews[0];
    realVertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    realVertexBufferView.StrideInBytes = vertexBufferStride;
    realVertexBufferView.SizeInBytes = vertexBufferSize;

    D3D12_VERTEX_BUFFER_VIEW& instanceBufferView = vertexBufferViews[1];
    instanceBufferView.BufferLocation = instanceBuffer->GetGPUVirtualAddress();
    instanceBufferView.StrideInBytes = instanceBufferStride;
    instanceBufferView.SizeInBytes = instanceBufferSize;

    D3D12_INDEX_BUFFER_VIEW indexBufferView;
    indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
    indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    indexBufferView.SizeInBytes = indexBufferSize;

    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_commandList->IASetVertexBuffers(0, 2, vertexBufferViews);
    m_commandList->IASetIndexBuffer(&indexBufferView);

    int numIndices = indexBufferSize / 4;

    int numInstances = instanceBufferSize / instanceBufferStride;

    // TODO: Check why instance count 2 doesn't work?
    for (int i = 0; i < numInstances; ++i) {
        m_commandList->DrawIndexedInstanced(numIndices, 1, 0, 0, i);
    }

#if false
    // Indicate that the back buffer will now be used to present.
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        m_commandList->ResourceBarrier(1, &barrier);
    }
#endif

    THROW_ERROR(
        m_commandList->Close(),
        "Can't close Command List!")

    return true;
}

bool rendering::deferred::DXLitMatCL::Execute(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, int signal, std::string& error)
{
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    commandQueue->Signal(fence, signal);

    return true;
}

bool rendering::deferred::DXLitMatCL::ExecuteAsync(ID3D12CommandQueue* commandQueue, std::string& error)
{
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    return true;
}

#undef THROW_ERROR