#include "materials/dxCloudMat.h"

#include "nativeFunc.h"

#include "d3dx12.h"

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
#include "dxDescriptorHeap.h"

void rendering::material::DXCloudMat::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

    Value& p_constantBuff = GetOrCreateProperty(nativeObject, "constantBuff");
    Value& p_vertexShader = GetOrCreateProperty(nativeObject, "vertexShader");
    Value& p_pixelShader = GetOrCreateProperty(nativeObject, "pixelShader");
    Value& p_noiseTextures = GetOrCreateProperty(nativeObject, "noiseTextures");
    Value& p_descriptorHeap = GetOrCreateProperty(nativeObject, "descriptorHeap");

    Value& setDescriptorHeap = GetOrCreateProperty(nativeObject, "setDescriptorHeap");
    setDescriptorHeap = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXCloudMat* self = static_cast<DXCloudMat*>(NativeObject::ExtractNativeObject(selfValue));

        Value descHeapValue = scope.GetProperty("param0");
        DXDescriptorHeap* descHeap = dynamic_cast<DXDescriptorHeap*>(NativeObject::ExtractNativeObject(descHeapValue));

        if (!descHeap) {
            THROW_EXCEPTION("Please supply Description Heap!")
        }

        p_descriptorHeap = descHeapValue;
        self->m_descriptorHeap = descHeap->GetHeap();

        return Value();
    });

    Value& addNoiseTexture = GetOrCreateProperty(nativeObject, "addNoiseTexture");
    addNoiseTexture = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXCloudMat* self = static_cast<DXCloudMat*>(NativeObject::ExtractNativeObject(selfValue));

        Value noiseTextureValue = scope.GetProperty("param0");
        DXTexture* noiseTexture = dynamic_cast<DXTexture*>(NativeObject::ExtractNativeObject(noiseTextureValue));

        if (!noiseTexture) {
            THROW_EXCEPTION("Please supply texture!")
        }

        std::list<Value> textures;
        if (!p_noiseTextures.IsNone()) {
            p_noiseTextures.ToList(textures);
        }
        textures.push_back(noiseTextureValue);

        p_noiseTextures = Value::FromList(textures);
        self->m_noiseTextures.push_back(noiseTexture->GetTexture());

        return Value();
    });

    Value& setConstantBuff = GetOrCreateProperty(nativeObject, "setConstantBuff");
    setConstantBuff = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXCloudMat* self = static_cast<DXCloudMat*>(NativeObject::ExtractNativeObject(selfValue));

        Value constantBuffValue = scope.GetProperty("param0");
        DXBuffer* constantBuff = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(constantBuffValue));

        if (!constantBuff) {
            THROW_EXCEPTION("Please supply constant buffer!")
        }

        p_constantBuff = constantBuffValue;
        self->m_constantBuffer = constantBuff->GetBuffer();

        return Value();
    });

    Value& setShaders = GetOrCreateProperty(nativeObject, "setShaders");
    setShaders = CreateNativeMethod(nativeObject, 2, [&](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXCloudMat* self = static_cast<DXCloudMat*>(NativeObject::ExtractNativeObject(selfValue));

        Value vertexShaderValue = scope.GetProperty("param0");
        DXVertexShader* vertexShader = dynamic_cast<DXVertexShader*>(NativeObject::ExtractNativeObject(vertexShaderValue));

        if (!vertexShader) {
            THROW_EXCEPTION("Please supply vertex shader!")
        }

        Value pixelShaderValue = scope.GetProperty("param1");
        DXPixelShader* pixelShader = dynamic_cast<DXPixelShader*>(NativeObject::ExtractNativeObject(pixelShaderValue));

        if (!pixelShader) {
            THROW_EXCEPTION("Please supply pixel shader!")
        }

        p_vertexShader = vertexShaderValue;
        p_pixelShader = pixelShaderValue;

        self->m_vertexShader = vertexShader->GetCompiledShader();
        self->m_pixelShader = pixelShader->GetCompiledShader();

        return Value();
    });

#undef THROW_EXCEPTION
}

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

rendering::material::MaterialType rendering::material::DXCloudMat::GetMaterialType() const
{
    return rendering::material::MaterialType::Cloud;
}

bool rendering::material::DXCloudMat::Render(
    rendering::DXRenderer* renderer,
    const DrawSettings& drawSettings,
    std::string& errorMessage)
{
    if (m_curRT == nullptr || m_curRT != renderer->GetISwapChain()->GetCurrentRenderTarget()) {
        THROW_ERROR(
            m_commandAllocator->Reset(),
            "Can't reset Command Allocator!")
    }

    m_curRT = renderer->GetISwapChain()->GetCurrentRenderTarget();

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    THROW_ERROR(
        m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()),
        "Can't reset Command List!")

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->SetDescriptorHeaps(1, &m_descriptorHeap);

    m_commandList->SetGraphicsRootConstantBufferView(0, renderer->GetCamBuff()->GetGPUVirtualAddress());
    m_commandList->SetGraphicsRootConstantBufferView(1, m_constantBuffer->GetGPUVirtualAddress());
    m_commandList->SetGraphicsRootDescriptorTable(2, m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

    m_commandList->RSSetViewports(1, &renderer->GetISwapChain()->m_viewport);
    m_commandList->RSSetScissorRects(1, &renderer->GetISwapChain()->m_scissorRect);

    D3D12_CPU_DESCRIPTOR_HANDLE dsHandle = renderer->GetDSHandle();
    D3D12_CPU_DESCRIPTOR_HANDLE handles[] = { renderer->GetISwapChain()->GetCurrentRTVDescriptor() };
    m_commandList->OMSetRenderTargets(_countof(handles), handles, FALSE, &dsHandle);

    D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[2];
    D3D12_VERTEX_BUFFER_VIEW& realVertexBufferView = vertexBufferViews[0];
    realVertexBufferView.BufferLocation = drawSettings.m_vertexBuffer->GetGPUVirtualAddress();
    realVertexBufferView.StrideInBytes = drawSettings.m_vertexBufferStride;
    realVertexBufferView.SizeInBytes = drawSettings.m_vertexBufferSize;

    D3D12_VERTEX_BUFFER_VIEW& instanceBufferView = vertexBufferViews[1];
    instanceBufferView.BufferLocation = drawSettings.m_instanceBuffer->GetGPUVirtualAddress();
    instanceBufferView.StrideInBytes = drawSettings.m_instanceBufferStride;
    instanceBufferView.SizeInBytes = drawSettings.m_instanceBufferSize;

    D3D12_INDEX_BUFFER_VIEW indexBufferView;
    indexBufferView.BufferLocation = drawSettings.m_indexBuffer->GetGPUVirtualAddress();
    indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    indexBufferView.SizeInBytes = drawSettings.m_indexBufferSize;

    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_commandList->IASetVertexBuffers(0, 2, vertexBufferViews);
    m_commandList->IASetIndexBuffer(&indexBufferView);

    int numIndices = drawSettings.m_indexBufferSize / 4;

    int numInstances = drawSettings.m_instanceBufferSize / drawSettings.m_instanceBufferStride;

    if (drawSettings.m_startIndexLocation < 0) {
        // TODO: Check why instance count 2 doesn't work?
        for (int i = 0; i < numInstances; ++i) {
            m_commandList->DrawIndexedInstanced(numIndices, 1, 0, 0, i);
        }
    }
    else {
        m_commandList->DrawIndexedInstanced(
            numIndices,
            1,
            drawSettings.m_startIndexLocation,
            0,
            drawSettings.m_startInstanceLocation
        );
    }

    THROW_ERROR(
        m_commandList->Close(),
        "Can't close Command List!")

    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    renderer->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    return true;
}

bool rendering::material::DXCloudMat::Init(DXRenderer& renderer, std::string& errorMessage)
{
    using Microsoft::WRL::ComPtr;
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(renderer.GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_ANISOTROPIC;
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

        CD3DX12_DESCRIPTOR_RANGE1 range;
        range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        CD3DX12_ROOT_PARAMETER1 rootParameters[3];
        rootParameters[0].InitAsConstantBufferView(0, 0);
        rootParameters[1].InitAsConstantBufferView(1, 0);
        rootParameters[2].InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL);

        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, rootSignatureFlags);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        THROW_ERROR(
            D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error),
            "Can't serialize a root signature!")

        THROW_ERROR(
            renderer.GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)),
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
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vertexShader);
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_pixelShader);
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

        psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
        psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND::D3D12_BLEND_SRC_ALPHA;
        psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND::D3D12_BLEND_INV_SRC_ALPHA;
        psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;
        psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;
        psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND::D3D12_BLEND_ONE;
        psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND::D3D12_BLEND_ZERO;
        psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = 0x0f;
        
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.SampleDesc.Count = 1;
        THROW_ERROR(
            renderer.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)),
            "Can't create Graphics Pipeline State!")
    }

    THROW_ERROR(
        renderer.GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)),
        "Can't create Command Allocator!")

    THROW_ERROR(
        renderer.GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)),
        "Can't create Command List!")

    THROW_ERROR(
        m_commandList->Close(),
        "Can't close command List!")

    return true;
}


#undef THROW_ERROR