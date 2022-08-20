#include "dxHeap.h"

#include "nativeFunc.h"

void rendering::DXHeap::InitProperties(interpreter::NativeObject& nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(message)\
scope.SetProperty("exception", Value(message));\
return Value();\

	Value& create = GetOrCreateProperty(nativeObject, "create");
	create = CreateNativeMethod(nativeObject, 2, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXHeap* heap = static_cast<DXHeap*>(NativeObject::ExtractNativeObject(selfValue));

		Value deviceValue = scope.GetProperty("param0");
		DXDevice* device = dynamic_cast<DXDevice*>(NativeObject::ExtractNativeObject(deviceValue));

		if (!device) {
			THROW_EXCEPTION("Please supply a Device!")
		}

		HRESULT hr = device->GetDevice().QueryInterface(IID_PPV_ARGS(&heap->m_device3));
		if (FAILED(hr)) {
			THROW_EXCEPTION("Can't Query ID3D12Device3!")
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

		
		std::string error;
		bool res = heap->Init(*device, desc, error);
		if (!res) {
			THROW_EXCEPTION(error);
		}

		return Value();
	});

	Value& makeResident = GetOrCreateProperty(nativeObject, "makeResident");
	makeResident = CreateNativeMethod(nativeObject, 2, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXHeap* heap = static_cast<DXHeap*>(NativeObject::ExtractNativeObject(selfValue));

		Value fenceValue = scope.GetProperty("param0");
		DXFence* fence = dynamic_cast<DXFence*>(NativeObject::ExtractNativeObject(fenceValue));

		if (!fence) {
			THROW_EXCEPTION("Please supply a Fence!");
		}

		Value signalValue = scope.GetProperty("param1");
		if (signalValue.GetType() != ScriptingValueType::Number) {
			THROW_EXCEPTION("Please supply a Singnal Value!");
		}

		int signal = signalValue.GetNum();
		if (signal < 0) {
			THROW_EXCEPTION("Please supply a valid Singnal Value!");
		}

		std::string error;
		bool res = heap->MakeResident(fence, signal, error);
		if (!res) {
			THROW_EXCEPTION(error);
		}
		return Value();
	});

#undef THROW_EXCEPTION

}

#define THROW_ERROR(hr, error)\
if (FAILED(hr)) {\
	errorMessage = error;\
	return false;\
}

bool rendering::DXHeap::Init(DXDevice& device, const D3D12_HEAP_DESC& desc, std::string errorMessage)
{

	THROW_ERROR(
		device.GetDevice().CreateHeap(&desc, IID_PPV_ARGS(&m_heap)),
		"Can't create Heap!")


	return true;
}

bool rendering::DXHeap::MakeResident(DXFence* fence, int signalValue, std::string& errorMessage)
{
	ID3D12Pageable* const tmp = m_heap.Get();

	THROW_ERROR(
		m_device3->EnqueueMakeResident(D3D12_RESIDENCY_FLAGS::D3D12_RESIDENCY_FLAG_DENY_OVERBUDGET, 1, &tmp, fence->GetFence(), signalValue),
		"Can't make the heap resident!")

	return true;
}

#undef THROW_ERROR