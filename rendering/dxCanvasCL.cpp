#include "dxCanvasCL.h"

#include "nativeFunc.h"
#include "dxDevice.h"
#include "dxVertexShader.h"
#include "dxPixelShader.h"
#include "dxSwapChain.h"
#include "dxBuffer.h"
#include "dxDescriptorHeap.h"

void rendering::DXCanvasCL::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

    Value& create = GetOrCreateProperty(nativeObject, "create");
    create = CreateNativeMethod(nativeObject, 5, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXCanvasCL* commandList = dynamic_cast<DXCanvasCL*>(NativeObject::ExtractNativeObject(selfValue));

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

        Value vertexBufferValue = scope.GetProperty("param3");
        DXBuffer* vertexBuffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(vertexBufferValue));

        if (!vertexBuffer) {
            THROW_EXCEPTION("Please supply a Vertex Buffer!")
        }

        Value vertexBufferStrideValue = scope.GetProperty("param4");
        if (vertexBufferStrideValue.GetType() != ScriptingValueType::Number) {
            THROW_EXCEPTION("Please supply a Stride!")
        }
        int stride = vertexBufferStrideValue.GetNum();
        if (stride <= 0) {
            THROW_EXCEPTION("Please supply a valid Stride!")
        }

        std::string error;
        bool res = commandList->Create(
            &device->GetDevice(),
            vertexShader->GetCompiledShader(),
            pixelShader->GetCompiledShader(),
            vertexBuffer->GetBuffer(),
            vertexBuffer->GetBufferWidth(),
            stride,
            error);

        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

    Value& populate = GetOrCreateProperty(nativeObject, "populate");
    populate = CreateNativeMethod(nativeObject, 2, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXCanvasCL* commandList = dynamic_cast<DXCanvasCL*>(NativeObject::ExtractNativeObject(selfValue));

        Value swapChainValue = scope.GetProperty("param0");
        DXSwapChain* swapChain = dynamic_cast<DXSwapChain*>(NativeObject::ExtractNativeObject(swapChainValue));

        if (!swapChain) {
            THROW_EXCEPTION("Please supply a swap chain!")
        }

        Value cbvHeapValue = scope.GetProperty("param1");
        DXDescriptorHeap* cbvHeap = dynamic_cast<DXDescriptorHeap*>(NativeObject::ExtractNativeObject(cbvHeapValue));

        if (!cbvHeap) {
            THROW_EXCEPTION("Please supply a constant buffer heap!")
        }

        std::string error;
        bool res = commandList->Populate(
            &swapChain->m_viewport,
            &swapChain->m_scissorRect,
            swapChain->GetCurrentRTVDescriptor(),
            swapChain->GetCurrentRenderTarget(),
            &commandList->m_vertexBufferView,
            cbvHeap->GetHeap(),
            error);

        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

    Value& execute = GetOrCreateProperty(nativeObject, "execute");
    execute = CreateNativeMethod(nativeObject, 3, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXCanvasCL* commandList = dynamic_cast<DXCanvasCL*>(NativeObject::ExtractNativeObject(selfValue));

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

bool rendering::DXCanvasCL::Create(
    ID3D12Device* device,
    ID3DBlob* vertexShader,
    ID3DBlob* pixelShader,
    ID3D12Resource* vertexBuffer,
    int vertexBufferWidth,
    int vertexBufferStride,
    std::string& errorMessage)
{
    using Microsoft::WRL::ComPtr;

    // Create the vertex buffer.
    {
        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = vertexBufferStride;
        m_vertexBufferView.SizeInBytes = vertexBufferWidth;
    }

    // Create a root signature consisting of a descriptor table with a single CBV.
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        CD3DX12_ROOT_PARAMETER1 rootParameters[1];

        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);

        // Allow input layout and deny uneccessary access to certain pipeline stages.
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(0, nullptr, 0, nullptr, rootSignatureFlags);

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

bool rendering::DXCanvasCL::Populate(
    const CD3DX12_VIEWPORT* viewport,
    CD3DX12_RECT* scissorRect,
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle,
    ID3D12Resource* renderTarget,
    const D3D12_VERTEX_BUFFER_VIEW* vertexBufferView,
    ID3D12DescriptorHeap* cbvHeap,
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

    //ID3D12DescriptorHeap* ppHeaps[] = { cbvHeap };
    //m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    //m_commandList->SetGraphicsRootDescriptorTable(0, cbvHeap->GetGPUDescriptorHandleForHeapStart());

    m_commandList->RSSetViewports(1, viewport);
    m_commandList->RSSetScissorRects(1, scissorRect);

#if false
    // Indicate that the back buffer will be used as a render target.
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_commandList->ResourceBarrier(1, &barrier);
    }
#endif 


    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, vertexBufferView);
    m_commandList->DrawInstanced(6, 1, 0, 0);

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

bool rendering::DXCanvasCL::Execute(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, int signal, std::string& error)
{
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    commandQueue->Signal(fence, signal);

    return true;
}

ID3D12GraphicsCommandList* rendering::DXCanvasCL::GetCommandList() const
{
    return m_commandList.Get();
}


#undef THROW_ERROR