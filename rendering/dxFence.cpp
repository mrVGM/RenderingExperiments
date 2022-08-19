#include "dxFence.h"

#include "nativeFunc.h"
#include "dxDevice.h"

void rendering::DXFence::InitProperties(interpreter::NativeObject& nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

	Value& create = GetOrCreateProperty(nativeObject, "create");
	create = CreateNativeMethod(nativeObject, 1, [](Value scope) {
		Value self = scope.GetProperty("self");
		DXFence& fence = static_cast<DXFence&>(*NativeObject::ExtractNativeObject(self));

		Value deviceValue = scope.GetProperty("param0");
		DXDevice* device = dynamic_cast<DXDevice*>(NativeObject::ExtractNativeObject(deviceValue));
		if (!device) {
			THROW_EXCEPTION("Please supply a device!")
		}

		HRESULT hr = device->GetDevice().CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence.m_fence));
		if (FAILED(hr)) {
			THROW_EXCEPTION("Can't create Fence!")
		}
		return Value();
	});

	Value& getEventID = GetOrCreateProperty(nativeObject, "getEventID");
	getEventID = CreateNativeMethod(nativeObject, 0, [](Value scope) {
		Value self = scope.GetProperty("self");
		DXFence* fence = static_cast<DXFence*>(NativeObject::ExtractNativeObject(self));

		return Value(fence->GetEventID());
	});

#undef THROW_EXCEPTION
}

ID3D12Fence* rendering::DXFence::GetFence() const
{
	return m_fence.Get();
}

int rendering::DXFence::GetEventID()
{
	m_eventCounterMutex.lock();
	int res = m_eventCounter++;
	m_eventCounterMutex.unlock();

	return res;
}
