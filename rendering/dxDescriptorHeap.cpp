#include "dxDescriptorHeap.h"

#include "d3dx12.h"
#include "nativeFunc.h"
#include "dxDevice.h"
#include "dxBuffer.h"
#include "dxTexture.h"

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

bool rendering::DXDescriptorHeap::Create(
    ID3D12Device* device,
    const std::list<interpreter::Value>& buffers,
    std::string& errorMessage)
{
    using namespace interpreter;

    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
    cbvHeapDesc.NumDescriptors = buffers.size();
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    THROW_ERROR(
        device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_heap)),
        "Can't create a descriptor heap")

    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_heap->GetCPUDescriptorHandleForHeapStart());
    UINT incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

#define EXCEPTION(message)\
errorMessage = message;\
return false;

    for (std::list<Value>::const_iterator i = buffers.begin(); i != buffers.end(); ++i) {
        const Value& cur = *i;

        if (cur.GetType() != ScriptingValueType::Object) {
            EXCEPTION("Invalid Descriptor!")
        }
        IManagedValue* obj = cur.GetManagedValue();
        Value type = obj->GetProperty("type");
        if (type.GetType() != ScriptingValueType::String) {
            EXCEPTION("Invalid Descriptor!")
        }

        bool processed = false;

        if (type.GetString() == "cbv") {
            processed = true;
            Value buffValue = obj->GetProperty("buffer");
            DXBuffer* buff = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(buffValue));
            if (!buff) {
                EXCEPTION("Invalid Descriptor!")
            }

            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
            cbvDesc.BufferLocation = buff->GetBuffer()->GetGPUVirtualAddress();
            cbvDesc.SizeInBytes = buff->GetBufferWidth();

            device->CreateConstantBufferView(&cbvDesc, handle);
        }

        if (type.GetString() == "srv_buff") {
            processed = true;
            Value buffValue = obj->GetProperty("buffer");
            DXBuffer* buff = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(buffValue));
            if (!buff) {
                EXCEPTION("Invalid Descriptor!")
            }

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = DXGI_FORMAT_UNKNOWN;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srvDesc.Buffer.FirstElement = 0;
            srvDesc.Buffer.NumElements = buff->GetElementCount();
            srvDesc.Buffer.StructureByteStride = buff->GetStride();
            srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

            device->CreateShaderResourceView(buff->GetBuffer(), &srvDesc, handle);
        }

        if (type.GetString() == "uav_buff") {
            processed = true;
            Value buffValue = obj->GetProperty("buffer");
            DXBuffer* buff = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(buffValue));
            if (!buff) {
                EXCEPTION("Invalid Descriptor!")
            }

            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = DXGI_FORMAT_UNKNOWN;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            uavDesc.Buffer.FirstElement = 0;
            uavDesc.Buffer.NumElements = buff->GetElementCount();
            uavDesc.Buffer.StructureByteStride = buff->GetStride();
            uavDesc.Buffer.CounterOffsetInBytes = 0;
            uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

            device->CreateUnorderedAccessView(buff->GetBuffer(), nullptr, &uavDesc, handle);
        }

        if (type.GetString() == "srv_tex") {
            processed = true;
            Value texValue = obj->GetProperty("texture");
            DXTexture* tex = dynamic_cast<DXTexture*>(NativeObject::ExtractNativeObject(texValue));
            if (!tex) {
                EXCEPTION("Invalid Descriptor!")
            }

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = tex->GetFormat();
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

            if (tex->GetTextureDimension() == 3) {
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            }

            srvDesc.Texture3D.MipLevels = 1;
            srvDesc.Texture2D.MipLevels = 1;

            device->CreateShaderResourceView(tex->GetTexture(), &srvDesc, handle);
        }

        if (type.GetString() == "uav_tex") {
            processed = true;
            Value texValue = obj->GetProperty("texture");
            DXTexture* tex = dynamic_cast<DXTexture*>(NativeObject::ExtractNativeObject(texValue));
            if (!tex) {
                EXCEPTION("Invalid Descriptor!")
            }

            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = tex->GetFormat();
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

            if (tex->GetTextureDimension() == 3) {
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
                uavDesc.Texture3D.WSize = -1;
            }

            device->CreateUnorderedAccessView(tex->GetTexture(), nullptr, &uavDesc, handle);
        }

        if (!processed) {
            EXCEPTION("Invalid Descriptor!")
        }

        handle.Offset(incrementSize);
    }
	return true;
#undef EXCEPTION
}

#undef THROW_ERROR

void rendering::DXDescriptorHeap::InitProperties(interpreter::NativeObject& nativeObject)
{
    using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

    Value& create = GetOrCreateProperty(nativeObject, "create");
    create = CreateNativeMethod(nativeObject, 2, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXDescriptorHeap* heap = static_cast<DXDescriptorHeap*>(NativeObject::ExtractNativeObject(selfValue));

        Value deviceValue = scope.GetProperty("param0");
        DXDevice* device = dynamic_cast<DXDevice*>(NativeObject::ExtractNativeObject(deviceValue));
        if (!device) {
            THROW_EXCEPTION("Please supply a device!")
        }

        Value bufferListValue = scope.GetProperty("param1");
        std::list<Value> bufferList;
        bufferListValue.ToList(bufferList);

        if (bufferList.size() == 0) {
            THROW_EXCEPTION("Please supply list of buffers!")
        }

        std::string error;
        bool res = heap->Create(&device->GetDevice(), bufferList, error);
        if (!res) {
            THROW_EXCEPTION(error)
        }

        return Value();
    });

#undef THROW_EXCEPTION
}

ID3D12DescriptorHeap* rendering::DXDescriptorHeap::GetHeap() const
{
    return m_heap.Get();
}
