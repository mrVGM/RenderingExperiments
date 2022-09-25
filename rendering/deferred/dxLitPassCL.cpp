#include "deferred/dxLitPassCL.h"

#include "nativeFunc.h"

#include "dxDevice.h"
#include "dxDescriptorHeap.h"
#include "dxCommandQueue.h"
#include "dxFence.h"
#include "dxTexture.h"
#include "deferred/gBuffer.h"
#include "dxPixelShader.h"
#include "dxVertexShader.h"
#include "dxBuffer.h"
#include "dxSwapChain.h"

#include <vector>

void rendering::deferred::DXLitPassCL::InitProperties(interpreter::NativeObject& nativeObject)
{
    using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

    Value& setupStartCL = GetOrCreateProperty(nativeObject, "setupStartCL");
    setupStartCL = CreateNativeMethod(nativeObject, 2, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXLitPassCL* self = static_cast<DXLitPassCL*>(NativeObject::ExtractNativeObject(selfValue));

        Value deviceValue = scope.GetProperty("param0");
        DXDevice* device = dynamic_cast<DXDevice*>(NativeObject::ExtractNativeObject(deviceValue));

        if (!device) {
            THROW_EXCEPTION("Please supply a device!")
        }

        Value gBufferValue = scope.GetProperty("param1");
        GBuffer* gBuffer = dynamic_cast<GBuffer*>(NativeObject::ExtractNativeObject(gBufferValue));

        if (!gBuffer) {
            THROW_EXCEPTION("Please supply a GBuffer!")
        }


        std::string error;
        bool res = self->SetupStartCL(
            &device->GetDevice(),
            gBuffer->GetDescriptorHandleFor(GBuffer::GBuffer_Diffuse),
            gBuffer->GetDescriptorHandleFor(GBuffer::GBuffer_Normal),
            gBuffer->GetDescriptorHandleFor(GBuffer::GBuffer_Position),
            gBuffer->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart(),
            gBuffer->GetTexture(GBuffer::GBuffer_Diffuse),
            gBuffer->GetTexture(GBuffer::GBuffer_Normal),
            gBuffer->GetTexture(GBuffer::GBuffer_Position),
            error
        );

        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

    Value& setupEndCL = GetOrCreateProperty(nativeObject, "setupEndCL");
    setupEndCL = CreateNativeMethod(nativeObject, 2, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXLitPassCL* self = static_cast<DXLitPassCL*>(NativeObject::ExtractNativeObject(selfValue));

        Value deviceValue = scope.GetProperty("param0");
        DXDevice* device = dynamic_cast<DXDevice*>(NativeObject::ExtractNativeObject(deviceValue));

        if (!device) {
            THROW_EXCEPTION("Please supply a device!")
        }

        Value gBufferValue = scope.GetProperty("param1");
        GBuffer* gBuffer = dynamic_cast<GBuffer*>(NativeObject::ExtractNativeObject(gBufferValue));

        if (!gBuffer) {
            THROW_EXCEPTION("Please supply a GBuffer!")
        }

        DXTexture* diffuseTex = dynamic_cast<DXTexture*>(NativeObject::ExtractNativeObject(gBufferValue.GetManagedValue()->GetProperty("diffuseTexture")));

        DXVertexShader* vertexShader = dynamic_cast<DXVertexShader*>(NativeObject::ExtractNativeObject(gBufferValue.GetManagedValue()->GetProperty("vertexShader")));
        DXPixelShader* pixelShader = dynamic_cast<DXPixelShader*>(NativeObject::ExtractNativeObject(gBufferValue.GetManagedValue()->GetProperty("pixelShader")));
        DXBuffer* vertexBuffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(gBufferValue.GetManagedValue()->GetProperty("vertexBuffer")));

        std::string error;
        bool res = self->SetupEndCL(
            &device->GetDevice(),
            vertexShader->GetCompiledShader(),
            pixelShader->GetCompiledShader(),
            vertexBuffer->GetBuffer(),
            vertexBuffer->GetBufferWidth(),
            vertexBuffer->GetStride(),
            error
        );

        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

    Value& populateEnd = GetOrCreateProperty(nativeObject, "populateEnd");
    populateEnd = CreateNativeMethod(nativeObject, 2, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXLitPassCL* self = static_cast<DXLitPassCL*>(NativeObject::ExtractNativeObject(selfValue));

        Value swapChainValue = scope.GetProperty("param0");
        DXSwapChain* swapChain = dynamic_cast<DXSwapChain*>(NativeObject::ExtractNativeObject(swapChainValue));

        if (!swapChain) {
            THROW_EXCEPTION("Please supply SwapChain!")
        }

        Value gBufferValue = scope.GetProperty("param1");
        GBuffer* gBuffer = dynamic_cast<GBuffer*>(NativeObject::ExtractNativeObject(gBufferValue));

        if (!gBuffer) {
            THROW_EXCEPTION("Please supply a GBuffer!")
        }

        DXVertexShader* vertexShader = dynamic_cast<DXVertexShader*>(NativeObject::ExtractNativeObject(gBufferValue.GetManagedValue()->GetProperty("vertexShader")));
        DXPixelShader* pixelShader = dynamic_cast<DXPixelShader*>(NativeObject::ExtractNativeObject(gBufferValue.GetManagedValue()->GetProperty("pixelShader")));
        DXBuffer* vertexBuffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(gBufferValue.GetManagedValue()->GetProperty("vertexBuffer")));
        DXDescriptorHeap* descriptorHeap = dynamic_cast<DXDescriptorHeap*>(NativeObject::ExtractNativeObject(gBufferValue.GetManagedValue()->GetProperty("descriptorHeap")));
        
        std::string error;
        bool res = self->PopulateEnd(
            &swapChain->m_viewport,
            &swapChain->m_scissorRect,
            swapChain->GetCurrentRTVDescriptor(),
            swapChain->GetCurrentRenderTarget(),
            &self->m_vertexBufferView,
            gBuffer->GetTexture(GBuffer::GBuffer_Diffuse),
            gBuffer->GetTexture(GBuffer::GBuffer_Normal),
            gBuffer->GetTexture(GBuffer::GBuffer_Position),
            descriptorHeap->GetHeap(),
            error
        );

        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

    Value& executeStart = GetOrCreateProperty(nativeObject, "executeStart");
    executeStart = CreateNativeMethod(nativeObject, 3, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXLitPassCL* self = static_cast<DXLitPassCL*>(NativeObject::ExtractNativeObject(selfValue));

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
        bool res = self->ExecuteStart(commandQueue->GetCommandQueue(), fence->GetFence(), signal, error);
        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

    Value& executeEnd = GetOrCreateProperty(nativeObject, "executeEnd");
    executeEnd = CreateNativeMethod(nativeObject, 3, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXLitPassCL* self = static_cast<DXLitPassCL*>(NativeObject::ExtractNativeObject(selfValue));

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
        bool res = self->ExecuteEnd(commandQueue->GetCommandQueue(), fence->GetFence(), signal, error);
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

bool rendering::deferred::DXLitPassCL::SetupStartCL(
    ID3D12Device* device,
    D3D12_CPU_DESCRIPTOR_HANDLE diffuseHandle,
    D3D12_CPU_DESCRIPTOR_HANDLE normalHandle,
    D3D12_CPU_DESCRIPTOR_HANDLE positionHandle,
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle,
    ID3D12Resource* diffuseTexture,
    ID3D12Resource* normalTexture,
    ID3D12Resource* positionTexture,
    std::string& errorMessage)
{
    using Microsoft::WRL::ComPtr;

    THROW_ERROR(
        device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocatorStart)),
        "Can't create Command Allocator Start!")

    THROW_ERROR(
        device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocatorStart.Get(), nullptr, IID_PPV_ARGS(&m_commandListStart)),
        "Can't create Command List Start!")

    {
        CD3DX12_RESOURCE_BARRIER barriers[] = {
            CD3DX12_RESOURCE_BARRIER::CD3DX12_RESOURCE_BARRIER::Transition(diffuseTexture, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET),
            CD3DX12_RESOURCE_BARRIER::CD3DX12_RESOURCE_BARRIER::Transition(normalTexture, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET),
            CD3DX12_RESOURCE_BARRIER::CD3DX12_RESOURCE_BARRIER::Transition(positionTexture, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET), 
        };
        m_commandListStart->ResourceBarrier(3, barriers);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[] = { diffuseHandle, normalHandle, positionHandle };
    m_commandListStart->OMSetRenderTargets(3, rtvHandles, FALSE, &dsvHandle);

    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    const float blackColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_commandListStart->ClearRenderTargetView(diffuseHandle, clearColor, 0, nullptr);
    m_commandListStart->ClearRenderTargetView(normalHandle, blackColor, 0, nullptr);
    m_commandListStart->ClearRenderTargetView(positionHandle, blackColor, 0, nullptr);

    m_commandListStart->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    THROW_ERROR(
        m_commandListStart->Close(),
        "Can't close command List!")

    return true;
}

bool rendering::deferred::DXLitPassCL::SetupEndCL(
    ID3D12Device* device,
    ID3DBlob* vertexShader,
    ID3DBlob* pixelShader,
    ID3D12Resource* vertexBuffer,
    int vertexBufferWidth,
    int vertexBufferStride,
    std::string& errorMessage)
{
    using Microsoft::WRL::ComPtr;

    // Create the vertex buffer view.
    {
        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = vertexBufferStride;
        m_vertexBufferView.SizeInBytes = vertexBufferWidth;
    }

    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        // Allow input layout and deny uneccessary access to certain pipeline stages.
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
        CD3DX12_ROOT_PARAMETER1 rootParameters[1];
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(1, rootParameters, 1, &sampler, rootSignatureFlags);

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
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        THROW_ERROR(
            device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)),
            "Can't create Graphics Pipeline State!")
    }

    THROW_ERROR(
        device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocatorEnd)),
        "Can't create Command Allocator!")

    THROW_ERROR(
        device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocatorEnd.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandListEnd)),
        "Can't create Command List!")

    THROW_ERROR(
        m_commandListEnd->Close(),
        "Can't close command List!")

    return true;
}

bool rendering::deferred::DXLitPassCL::PopulateEnd(
    const CD3DX12_VIEWPORT* viewport,
    CD3DX12_RECT* scissorRect,
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle,
    ID3D12Resource* renderTarget,
    const D3D12_VERTEX_BUFFER_VIEW* vertexBufferView,
    ID3D12Resource* diffuseTex,
    ID3D12Resource* normalTex,
    ID3D12Resource* positionTex,
    ID3D12DescriptorHeap* descriptorHeap,
    std::string& errorMessage)
{
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    THROW_ERROR(
        m_commandAllocatorEnd->Reset(),
        "Can't reset Command Allocator End!")

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    THROW_ERROR(
        m_commandListEnd->Reset(m_commandAllocatorEnd.Get(), m_pipelineState.Get()),
        "Can't reset Command List End!")

    // Textures are now SRV
    {
        CD3DX12_RESOURCE_BARRIER barriers[] = {
            CD3DX12_RESOURCE_BARRIER::CD3DX12_RESOURCE_BARRIER::Transition(diffuseTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
            CD3DX12_RESOURCE_BARRIER::CD3DX12_RESOURCE_BARRIER::Transition(normalTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
            CD3DX12_RESOURCE_BARRIER::CD3DX12_RESOURCE_BARRIER::Transition(positionTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
        };
        m_commandListEnd->ResourceBarrier(3, barriers);
    }

    // Set necessary state.
    m_commandListEnd->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandListEnd->SetDescriptorHeaps(1, &descriptorHeap);
    m_commandListEnd->SetGraphicsRootDescriptorTable(0, descriptorHeap->GetGPUDescriptorHandleForHeapStart());


    // Indicate that the back buffer will be used as a render target.
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_commandListEnd->ResourceBarrier(1, &barrier);
    }
    m_commandListEnd->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    const float clearColor[] = { 1.0f, 0.2f, 0.4f, 1.0f };
    m_commandListEnd->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    m_commandListEnd->RSSetViewports(1, viewport);
    m_commandListEnd->RSSetScissorRects(1, scissorRect);

    m_commandListEnd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandListEnd->IASetVertexBuffers(0, 1, vertexBufferView);
    m_commandListEnd->DrawInstanced(6, 1, 0, 0);

    {
        CD3DX12_RESOURCE_BARRIER barriers[] = {
            CD3DX12_RESOURCE_BARRIER::CD3DX12_RESOURCE_BARRIER::Transition(diffuseTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PRESENT),
            CD3DX12_RESOURCE_BARRIER::CD3DX12_RESOURCE_BARRIER::Transition(normalTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PRESENT),
            CD3DX12_RESOURCE_BARRIER::CD3DX12_RESOURCE_BARRIER::Transition(positionTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PRESENT),
        };
        m_commandListEnd->ResourceBarrier(3, barriers);
    }

    // Indicate that the back buffer will now be used to present.
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        m_commandListEnd->ResourceBarrier(1, &barrier);
    }

    THROW_ERROR(
        m_commandListEnd->Close(),
        "Can't close Command List End!")

    return true;
}

bool rendering::deferred::DXLitPassCL::ExecuteStart(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, int signal, std::string& error)
{
    ID3D12CommandList* ppCommandLists[] = { m_commandListStart.Get() };
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    commandQueue->Signal(fence, signal);

    return true;
}

bool rendering::deferred::DXLitPassCL::ExecuteEnd(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, int signal, std::string& error)
{
    ID3D12CommandList* ppCommandLists[] = { m_commandListEnd.Get() };
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    commandQueue->Signal(fence, signal);

    return true;
}

#undef THROW_ERROR