#include "dxCommandList.h"

#include "d3dx12.h"
#include "nativeFunc.h"
#include "dxDevice.h"
#include "dxVertexShader.h"
#include "dxPixelShader.h"

void rendering::DXCommandList::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

    Value& create = GetOrCreateProperty(nativeObject, "create");
    create = CreateNativeMethod(nativeObject, 3, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXCommandList* commandList = dynamic_cast<DXCommandList*>(NativeObject::ExtractNativeObject(selfValue));

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

        std::string error;
        bool res = commandList->Create(&device->GetDevice(), vertexShader->GetCompiledShader(), pixelShader->GetCompiledShader(), error);
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

bool rendering::DXCommandList::Create(ID3D12Device* device, ID3DBlob* vertexShader, ID3DBlob* pixelShader, std::string& errorMessage)
{
    using Microsoft::WRL::ComPtr;

    // Create an empty root signature.
    {
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        THROW_ERROR(
            D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error),
            "Can't serialize Root Signature!")

            THROW_ERROR(
                device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)),
                "Can't Create Root Signature!")
    }


    // Create the pipeline state, which includes compiling and loading shaders.
    {
        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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
        device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)),
        "Can't create Command Allocator!"
    )

    THROW_ERROR(
        device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)),
        "Can't create Command List!")

    return true;
}

#undef THROW_ERROR