#include "dxHeap.h"

#include "nativeFunc.h"

void rendering::DXHeap::InitProperties(interpreter::NativeObject& nativeObject)
{
	using namespace interpreter;
	Value& create = GetOrCreateProperty(nativeObject, "create");

#define THROW_EXCEPTION(message)\
scope.SetProperty("exception", Value(message));\
return Value();\


	create = CreateNativeMethod(nativeObject, 2, [](Value scope) {
		Value deviceValue = scope.GetProperty("param0");
		DXDevice* device = dynamic_cast<DXDevice*>(NativeObject::ExtractNativeObject(deviceValue));

		if (!device) {
			THROW_EXCEPTION("Please supply a Device!")
		}

		Value heapSizeValue = scope.GetProperty("param1");
		if (heapSizeValue.GetType() != ScriptingValueType::Number) {
			THROW_EXCEPTION("Please supply heap size!")
		}

		int size = (int) heapSizeValue.GetNum();
		if (size <= 0) {
			THROW_EXCEPTION("Please supply a valid heap size!")
		}

		D3D12_HEAP_DESC desc = {};
		desc.SizeInBytes = size;
		desc.Properties.Type = D3D12_HEAP_TYPE_UPLOAD;
		desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		desc.Properties.CreationNodeMask = 0;
		desc.Properties.VisibleNodeMask = 0;

		desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Flags =
			D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_CREATE_NOT_ZEROED |
			D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_CREATE_NOT_RESIDENT |
			D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;

		Value selfValue = scope.GetProperty("self");
		DXHeap* heap = static_cast<DXHeap*>(NativeObject::ExtractNativeObject(selfValue));

		std::string error;
		bool res = heap->Init(*device, desc, error);
		if (!res) {
			THROW_EXCEPTION(error);
		}

		return Value();
	});

#undef THROW_EXCEPTION

}

bool rendering::DXHeap::Init(DXDevice& device, const D3D12_HEAP_DESC& desc, std::string errorMessage)
{
#define THROW_ERROR(hr, error)\
if (FAILED(hr)) {\
	errorMessage = error;\
	return false;\
}\

	THROW_ERROR(
		device.GetDevice().CreateHeap(&desc, IID_PPV_ARGS(&m_heap)),
		"Can't create Heap!")

#undef THROW_ERROR

	return true;
}
