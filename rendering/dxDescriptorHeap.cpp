#include "dxDescriptorHeap.h"

#include "d3dx12.h"
#include "nativeFunc.h"
#include "dxDevice.h"

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

bool rendering::DXDescriptorHeap::Create(
    ID3D12Device* device,
    const std::vector<DXBuffer*>& constantBuffers,
    std::string& errorMessage)
{
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
    cbvHeapDesc.NumDescriptors = constantBuffers.size();
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    THROW_ERROR(
        device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_heap)),
        "Can't create a descriptor heap")

    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_heap->GetCPUDescriptorHandleForHeapStart());
    UINT incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    for (int i = 0; i < constantBuffers.size(); ++i) {
        DXBuffer* curBuffer = constantBuffers[i];

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = curBuffer->GetBuffer()->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = curBuffer->GetBufferWidth();

        device->CreateConstantBufferView(&cbvDesc, handle);
        handle.Offset(incrementSize);
    }

	return true;
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
        std::vector<Value> bufferList;
        bufferListValue.ToList(bufferList);

        if (bufferList.size() == 0) {
            THROW_EXCEPTION("Please supply list of buffers!")
        }

        std::vector<DXBuffer*> buffers;
        for (int i = 0; i < bufferList.size(); ++i) {
            Value tmp = bufferList[i];
            DXBuffer* buffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(tmp));

            if (!buffer) {
                THROW_EXCEPTION("Buffer list contains other kind of elements!")
            }
            buffers.push_back(buffer);
        }

        std::string error;
        bool res = heap->Create(&device->GetDevice(), buffers, error);
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
