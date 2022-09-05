#include "dxHeap.h"

#include "nativeFunc.h"

void rendering::DXHeap::InitProperties(interpreter::NativeObject& nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(message)\
scope.SetProperty("exception", Value(message));\
return Value();\

	Value& create = GetOrCreateProperty(nativeObject, "create");
	create = CreateNativeMethod(nativeObject, 3, [](Value scope) {
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

		Value heapTypeValue = scope.GetProperty("param2");
		if (heapTypeValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply heap type!")
		}

		std::string heapType = heapTypeValue.GetString();
		if (heapType != "DEFAULT" && heapType != "UPLOAD" && heapType != "READBACK") {
			THROW_EXCEPTION("Please supply a valid heap type!")
		}
		heap->m_heapType = heapType;

		D3D12_HEAP_TYPE type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
		if (heapType == "UPLOAD") {
			type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
		}
		if (heapType == "READBACK") {
			type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_READBACK;
		}


		D3D12_HEAP_DESC desc = {};
		desc.SizeInBytes = size;
		desc.Properties.Type = type;
		desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		desc.Properties.CreationNodeMask = 0;
		desc.Properties.VisibleNodeMask = 0;

		desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Flags =
			D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_CREATE_NOT_ZEROED |
			D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_CREATE_NOT_RESIDENT;

		
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

	Value& evict = GetOrCreateProperty(nativeObject, "evict");
	evict = CreateNativeMethod(nativeObject, 0, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXHeap* heap = static_cast<DXHeap*>(NativeObject::ExtractNativeObject(selfValue));

		std::string error;
		bool res = heap->Evict(error);
		if (!res) {
			THROW_EXCEPTION(error);
		}
		return Value();
	});

	Value& getHeapType = GetOrCreateProperty(nativeObject, "getHeapType");
	getHeapType = CreateNativeMethod(nativeObject, 0, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXHeap* heap = static_cast<DXHeap*>(NativeObject::ExtractNativeObject(selfValue));

		return Value(heap->m_heapType);
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
	if (m_resident) {
		errorMessage = "The heap is already Resident!";
		return false;
	}

	ID3D12Pageable* const tmp = m_heap.Get();

	THROW_ERROR(
		m_device3->EnqueueMakeResident(D3D12_RESIDENCY_FLAGS::D3D12_RESIDENCY_FLAG_DENY_OVERBUDGET, 1, &tmp, fence->GetFence(), signalValue),
		"Can't make the heap resident!")

	m_resident = true;

	return true;
}

bool rendering::DXHeap::Evict(std::string& errorMessage)
{
	if (!m_resident) {
		errorMessage = "The heap is not Resident yet!";
		return false;
	}

	ID3D12Pageable* const tmp = m_heap.Get();

	THROW_ERROR(
		m_device3->Evict(1, &tmp),
		"Can't Evict the Heap!")

	m_resident = false;

	return true;
}

ID3D12Heap* rendering::DXHeap::GetHeap() const
{
	return m_heap.Get();
}

const std::string& rendering::DXHeap::GetHeapType() const
{
	return m_heapType;
}

rendering::DXHeap::~DXHeap()
{
	if (m_resident) {
		std::string error;
		Evict(error);
	}
}

#undef THROW_ERROR