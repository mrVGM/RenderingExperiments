#include "dxCommandQueue.h"

#include "nativeFunc.h"
#include "dxDevice.h"

void rendering::DXCommandQueue::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

    Value& create = GetOrCreateProperty(nativeObject, "create");
    create = CreateNativeMethod(nativeObject, 1, [](Value scope) {
        Value selfValue = scope.GetProperty("self");
        DXCommandQueue* commandQueue = dynamic_cast<DXCommandQueue*>(NativeObject::ExtractNativeObject(selfValue));

        Value deviceValue = scope.GetProperty("param0");
        DXDevice* device = dynamic_cast<DXDevice*>(NativeObject::ExtractNativeObject(deviceValue));

        std::string error;
        bool res = commandQueue->Create(&device->GetDevice(), error);

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

bool rendering::DXCommandQueue::Create(ID3D12Device* device, std::string& errorMessage)
{
    using Microsoft::WRL::ComPtr;

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    THROW_ERROR(
        device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)),
        "Can't create Command QUEUE!")

    return true;
}

#undef THROW_ERROR