#include "deferred/gBuffer.h"

#include "nativeFunc.h"

#include "dxDevice.h"
#include "dxDescriptorHeap.h"
#include "dxCommandQueue.h"
#include "dxFence.h"
#include "dxTexture.h"
#include "dxBuffer.h"
#include "dxVertexShader.h"
#include "dxPixelShader.h"
#include "dxDescriptorHeap.h"

void rendering::deferred::GBuffer::InitProperties(interpreter::NativeObject& nativeObject)
{
    using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

    Value& width = GetOrCreateProperty(nativeObject, "width");
    Value& height = GetOrCreateProperty(nativeObject, "height");

    Value& dsTexture = GetOrCreateProperty(nativeObject, "dsTexture");
    Value& diffuseTexture = GetOrCreateProperty(nativeObject, "diffuseTexture");
    Value& normalTexture = GetOrCreateProperty(nativeObject, "normalTexture");
    Value& positionTexture = GetOrCreateProperty(nativeObject, "positionTexture");

    Value& vertexBuffer = GetOrCreateProperty(nativeObject, "vertexBuffer");
    Value& vertexShader = GetOrCreateProperty(nativeObject, "vertexShader");
    Value& pixelShader = GetOrCreateProperty(nativeObject, "pixelShader");
    Value& gBuffDescriptorHeap = GetOrCreateProperty(nativeObject, "descriptorHeap");

    Value& create = GetOrCreateProperty(nativeObject, "create");
    create = CreateNativeMethod(nativeObject, 11, [&](Value scope) {
        Value selfValue = scope.GetProperty("self");
        GBuffer* self = static_cast<GBuffer*>(NativeObject::ExtractNativeObject(selfValue));

        Value deviceValue = scope.GetProperty("param0");
        DXDevice* device = dynamic_cast<DXDevice*>(NativeObject::ExtractNativeObject(deviceValue));

        if (!device) {
            THROW_EXCEPTION("Please supply a device!")
        }

        Value vertexBufferValue = scope.GetProperty("param1");
        DXBuffer* dxVertexBuffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(vertexBufferValue));

        if (!dxVertexBuffer) {
            THROW_EXCEPTION("Please supply a Vertex Buffer!")
        }
        vertexBuffer = vertexBufferValue;

        Value vertexShaderValue = scope.GetProperty("param2");
        DXVertexShader* dxVertexShader = dynamic_cast<DXVertexShader*>(NativeObject::ExtractNativeObject(vertexShaderValue));

        if (!dxVertexShader) {
            THROW_EXCEPTION("Please supply a Vertex Shader!")
        }
        vertexShader = vertexShaderValue;

        Value pixelShaderValue = scope.GetProperty("param3");
        DXPixelShader* dxPixelShader = dynamic_cast<DXPixelShader*>(NativeObject::ExtractNativeObject(pixelShaderValue));

        if (!dxPixelShader) {
            THROW_EXCEPTION("Please supply a Pixel Shader!")
        }
        pixelShader = pixelShaderValue;

        Value widthValue = scope.GetProperty("param4");
        if (widthValue.GetType() != ScriptingValueType::Number) {
            THROW_EXCEPTION("Please supply a width value!")
        }
        width = widthValue;

        Value heightValue = scope.GetProperty("param5");
        if (heightValue.GetType() != ScriptingValueType::Number) {
            THROW_EXCEPTION("Please supply a height value!")
        }
        height = heightValue;

        Value dsTexValue = scope.GetProperty("param6");
        DXTexture* dsTex = dynamic_cast<DXTexture*>(NativeObject::ExtractNativeObject(dsTexValue));

        if (!dsTex) {
            THROW_EXCEPTION("Please supply a depth stencil texture!")
        }
        dsTexture = dsTexValue;

        Value gBuffDescHeapValue = scope.GetProperty("param7");
        DXDescriptorHeap* gBuffDescHeap = dynamic_cast<DXDescriptorHeap*>(NativeObject::ExtractNativeObject(gBuffDescHeapValue));

        if (!gBuffDescHeap) {
            THROW_EXCEPTION("Please supply a GBuffer Descriptor Heap!")
        }
        gBuffDescriptorHeap = gBuffDescHeapValue;

        Value diffuseTexValue = scope.GetProperty("param8");
        DXTexture* diffuseTex = dynamic_cast<DXTexture*>(NativeObject::ExtractNativeObject(diffuseTexValue));

        if (!diffuseTex) {
            THROW_EXCEPTION("Please supply a diffuse texture!")
        }
        diffuseTexture = diffuseTexValue;

        Value normalTexValue = scope.GetProperty("param9");
        DXTexture* normalTex = dynamic_cast<DXTexture*>(NativeObject::ExtractNativeObject(normalTexValue));

        if (!normalTex) {
            THROW_EXCEPTION("Please supply a normal texture!")
        }
        normalTexture = normalTexValue;

        Value positionTexValue = scope.GetProperty("param10");
        DXTexture* positionTex = dynamic_cast<DXTexture*>(NativeObject::ExtractNativeObject(positionTexValue));

        if (!positionTex) {
            THROW_EXCEPTION("Please supply a position texture!")
        }
        positionTexture = positionTexValue;

        std::string error;
        bool res = self->Create(
            &device->GetDevice(),
            dsTex->GetTexture(),
            diffuseTex->GetTexture(),
            error
        );

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

bool rendering::deferred::GBuffer::Create(
    ID3D12Device* device,
    ID3D12Resource* dsTexture,
    ID3D12Resource* diffuseTexture,
    std::string& errorMessage)
{
    using Microsoft::WRL::ComPtr;

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = 4;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        THROW_ERROR(
            device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)),
            "Can't create a descriptor heap!")

        m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }
    
    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        device->CreateRenderTargetView(diffuseTexture, nullptr, rtvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);
    }

    // Create DSV Heap
    {
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        THROW_ERROR(
            device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)),
            "Can't Create DSV Heap!"
        );

        m_dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    }

    {
        D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
        depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
        depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

        CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

        device->CreateDepthStencilView(dsTexture, &depthStencilDesc, dsvHandle);
    }
    
    
    return true;
}

ID3D12DescriptorHeap* rendering::deferred::GBuffer::GetRTVHeap() const
{
    return m_rtvHeap.Get();
}

ID3D12DescriptorHeap* rendering::deferred::GBuffer::GetDSVHeap() const
{
    return m_dsvHeap.Get();
}

#undef THROW_ERROR