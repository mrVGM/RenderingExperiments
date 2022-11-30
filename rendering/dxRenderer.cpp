#include "dxRenderer.h"

#include "nativeFunc.h"
#include "dxDevice.h"
#include "dxSwapChain.h"
#include "dxCommandQueue.h"


#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

void rendering::DXRenderer::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

	Value& p_device = GetOrCreateProperty(nativeObject, "device");
	Value& p_swapChain = GetOrCreateProperty(nativeObject, "swapChain");
	Value& p_commandQueue = GetOrCreateProperty(nativeObject, "commandQueue");
	
	Value& setDevice = GetOrCreateProperty(nativeObject, "setDevice");
	setDevice = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXRenderer* self = static_cast<DXRenderer*>(NativeObject::ExtractNativeObject(selfValue));

		Value deviceValue = scope.GetProperty("param0");
		DXDevice* device = dynamic_cast<DXDevice*>(NativeObject::ExtractNativeObject(deviceValue));

		if (!device) {
			THROW_EXCEPTION("Please supply a device!")
		}

		p_device = deviceValue;
		self->m_device = &device->GetDevice();

		return Value();
	});

	Value& setSwapChain = GetOrCreateProperty(nativeObject, "setSwapChain");
	setSwapChain = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXRenderer* self = static_cast<DXRenderer*>(NativeObject::ExtractNativeObject(selfValue));

		Value swapChainValue = scope.GetProperty("param0");
		DXSwapChain* swapChain = dynamic_cast<DXSwapChain*>(NativeObject::ExtractNativeObject(swapChainValue));

		if (!swapChain) {
			THROW_EXCEPTION("Please supply a SwapChain!")
		}

		p_swapChain = swapChainValue;
		self->m_swapChain = swapChain;

		return Value();
	});

	Value& setCommandQueue = GetOrCreateProperty(nativeObject, "setCommandQueue");
	setCommandQueue = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXRenderer* self = static_cast<DXRenderer*>(NativeObject::ExtractNativeObject(selfValue));

		Value commandQueueValue = scope.GetProperty("param0");
		DXCommandQueue* commandQueue = dynamic_cast<DXCommandQueue*>(NativeObject::ExtractNativeObject(commandQueueValue));

		if (!commandQueue) {
			THROW_EXCEPTION("Please supply a Command Queue!")
		}

		p_commandQueue = commandQueueValue;
		self->m_commandQueue = commandQueue->GetCommandQueue();

		return Value();
	});
	

#undef THROW_EXCEPTION
}

#undef THROW_ERROR