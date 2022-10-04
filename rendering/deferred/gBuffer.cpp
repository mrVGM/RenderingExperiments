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
    Value& specularTexture = GetOrCreateProperty(nativeObject, "specularTexture");

    Value& camBuffer = GetOrCreateProperty(nativeObject, "camBuffer");

    Value& vertexBuffer = GetOrCreateProperty(nativeObject, "vertexBuffer");
    Value& vertexShader = GetOrCreateProperty(nativeObject, "vertexShader");
    Value& pixelShader = GetOrCreateProperty(nativeObject, "pixelShader");
    Value& gBuffDescriptorHeap = GetOrCreateProperty(nativeObject, "descriptorHeap");

    Value& create = GetOrCreateProperty(nativeObject, "create");
    create = CreateNativeMethod(nativeObject, 13, [&](Value scope) {
        Value selfValue = scope.GetProperty("self");
        GBuffer* self = static_cast<GBuffer*>(NativeObject::ExtractNativeObject(selfValue));

        Value deviceValue = scope.GetProperty("param0");
        DXDevice* device = dynamic_cast<DXDevice*>(NativeObject::ExtractNativeObject(deviceValue));

        if (!device) {
            THROW_EXCEPTION("Please supply a device!")
        }

        Value camBuffValue = scope.GetProperty("param1");
        DXBuffer* camBuff = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(camBuffValue));

        if (!camBuff) {
            THROW_EXCEPTION("Please supply a Cam Buffer!")
        }
        m_camBuffer = camBuff->GetBuffer();
        camBuffer = camBuffValue;

        Value vertexBufferValue = scope.GetProperty("param2");
        DXBuffer* dxVertexBuffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(vertexBufferValue));

        if (!dxVertexBuffer) {
            THROW_EXCEPTION("Please supply a Vertex Buffer!")
        }
        vertexBuffer = vertexBufferValue;

        Value vertexShaderValue = scope.GetProperty("param3");
        DXVertexShader* dxVertexShader = dynamic_cast<DXVertexShader*>(NativeObject::ExtractNativeObject(vertexShaderValue));

        if (!dxVertexShader) {
            THROW_EXCEPTION("Please supply a Vertex Shader!")
        }
        vertexShader = vertexShaderValue;

        Value pixelShaderValue = scope.GetProperty("param4");
        DXPixelShader* dxPixelShader = dynamic_cast<DXPixelShader*>(NativeObject::ExtractNativeObject(pixelShaderValue));

        if (!dxPixelShader) {
            THROW_EXCEPTION("Please supply a Pixel Shader!")
        }
        pixelShader = pixelShaderValue;

        Value widthValue = scope.GetProperty("param5");
        if (widthValue.GetType() != ScriptingValueType::Number) {
            THROW_EXCEPTION("Please supply a width value!")
        }
        width = widthValue;

        Value heightValue = scope.GetProperty("param6");
        if (heightValue.GetType() != ScriptingValueType::Number) {
            THROW_EXCEPTION("Please supply a height value!")
        }
        height = heightValue;

        Value dsTexValue = scope.GetProperty("param7");
        DXTexture* dsTex = dynamic_cast<DXTexture*>(NativeObject::ExtractNativeObject(dsTexValue));

        if (!dsTex) {
            THROW_EXCEPTION("Please supply a depth stencil texture!")
        }
        dsTexture = dsTexValue;

        Value gBuffDescHeapValue = scope.GetProperty("param8");
        DXDescriptorHeap* gBuffDescHeap = dynamic_cast<DXDescriptorHeap*>(NativeObject::ExtractNativeObject(gBuffDescHeapValue));

        if (!gBuffDescHeap) {
            THROW_EXCEPTION("Please supply a GBuffer Descriptor Heap!")
        }
        gBuffDescriptorHeap = gBuffDescHeapValue;

        Value diffuseTexValue = scope.GetProperty("param9");
        DXTexture* diffuseTex = dynamic_cast<DXTexture*>(NativeObject::ExtractNativeObject(diffuseTexValue));

        if (!diffuseTex) {
            THROW_EXCEPTION("Please supply a diffuse texture!")
        }
        diffuseTexture = diffuseTexValue;
        m_diffuseTex = diffuseTex->GetTexture();

        Value normalTexValue = scope.GetProperty("param10");
        DXTexture* normalTex = dynamic_cast<DXTexture*>(NativeObject::ExtractNativeObject(normalTexValue));

        if (!normalTex) {
            THROW_EXCEPTION("Please supply a normal texture!")
        }
        normalTexture = normalTexValue;
        m_normalTex = normalTex->GetTexture();

        Value positionTexValue = scope.GetProperty("param11");
        DXTexture* positionTex = dynamic_cast<DXTexture*>(NativeObject::ExtractNativeObject(positionTexValue));

        if (!positionTex) {
            THROW_EXCEPTION("Please supply a position texture!")
        }
        positionTexture = positionTexValue;
        m_positionTex = positionTex->GetTexture();

        Value specularTexValue = scope.GetProperty("param12");
        DXTexture* specularTex = dynamic_cast<DXTexture*>(NativeObject::ExtractNativeObject(specularTexValue));

        if (!specularTex) {
            THROW_EXCEPTION("Please supply a specular texture!")
        }
        specularTexture = specularTexValue;
        m_specularTex = specularTex->GetTexture();

        std::string error;
        bool res = self->Create(
            &device->GetDevice(),
            dsTex->GetTexture(),
            diffuseTex->GetTexture(),
            normalTex->GetTexture(),
            positionTex->GetTexture(),
            specularTex->GetTexture(),
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
    ID3D12Resource* diffuseTex,
    ID3D12Resource* normalTex,
    ID3D12Resource* positionTex,
    ID3D12Resource* specularTex,
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

        device->CreateRenderTargetView(diffuseTex, nullptr, rtvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);

        device->CreateRenderTargetView(normalTex, nullptr, rtvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);

        device->CreateRenderTargetView(positionTex, nullptr, rtvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);

        device->CreateRenderTargetView(specularTex, nullptr, rtvHandle);
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

D3D12_CPU_DESCRIPTOR_HANDLE rendering::deferred::GBuffer::GetDescriptorHandleFor(GBuffTextureType texType)
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < texType; ++i) {
        handle.Offset(m_rtvDescriptorSize);
    }
    return handle;
}

ID3D12Resource* rendering::deferred::GBuffer::GetTexture(GBuffTextureType texType)
{
    switch (texType)
    {
    case GBuffer_Diffuse:
        return m_diffuseTex;
    case GBuffer_Normal:
        return m_normalTex;
    case GBuffer_Position:
        return m_positionTex;
    case GBuffer_Specular:
        return m_specularTex;
    }
    return nullptr;
}

ID3D12Resource* rendering::deferred::GBuffer::GetCamBuffer() const
{
    return m_camBuffer;
}

#undef THROW_ERROR