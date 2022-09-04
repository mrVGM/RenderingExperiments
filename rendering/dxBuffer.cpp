#include "dxBuffer.h"

#include "d3dx12.h"
#include "nativeFunc.h"
#include "dxDevice.h"
#include "dxHeap.h"

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

void rendering::DXBuffer::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

	Value& init = GetOrCreateProperty(nativeObject, "init");
	init = CreateNativeMethod(nativeObject, 1, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXBuffer* buffer = static_cast<DXBuffer*>(NativeObject::ExtractNativeObject(selfValue));

		Value widthValue = scope.GetProperty("param0");
		if (widthValue.GetType() != ScriptingValueType::Number) {
			THROW_EXCEPTION("Please supply buffer width!");
		}
		int width = widthValue.GetNum();
		if (width <= 0) {
			THROW_EXCEPTION("Please supply a valid buffer width!");
		}
		buffer->m_width = width;
		buffer->m_stride = width;

		return Value();
	});

	Value& place = GetOrCreateProperty(nativeObject, "place");
	place = CreateNativeMethod(nativeObject, 3, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXBuffer* buffer = static_cast<DXBuffer*>(NativeObject::ExtractNativeObject(selfValue));

		Value deviceValue = scope.GetProperty("param0");
		DXDevice* device = dynamic_cast<DXDevice*>(NativeObject::ExtractNativeObject(deviceValue));

		if (!device) {
			THROW_EXCEPTION("Please supply device!");
		}

		Value heapValue = scope.GetProperty("param1");
		DXHeap* heap = dynamic_cast<DXHeap*>(NativeObject::ExtractNativeObject(heapValue));
		if (!heap) {
			THROW_EXCEPTION("Please supply heap!");
		}

		Value heapOffset = scope.GetProperty("param2");
		if (heapOffset.GetType() != ScriptingValueType::Number) {
			THROW_EXCEPTION("Please supply heap offset!");
		}

		int offset = heapOffset.GetNum();
		if (offset < 0) {
			THROW_EXCEPTION("Please supply a valid heap offset!");
		}

		std::string error;
		bool res = buffer->Place(&device->GetDevice(), heap->GetHeap(), offset, buffer->m_width, error);
		if (!res) {
			THROW_EXCEPTION(error);
		}

		return Value();
	});

	Value& copyData = GetOrCreateProperty(nativeObject, "copyData");
	copyData = CreateNativeMethod(nativeObject, 1, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXBuffer* buffer = static_cast<DXBuffer*>(NativeObject::ExtractNativeObject(selfValue));

		Value data = scope.GetProperty("param0");
		std::vector<Value> list;
		data.ToList(list);
		if (list.size() == 0) {
			THROW_EXCEPTION("Please supply buffer data!")
		}

		for (int i = 0; i < list.size(); ++i) {
			const Value& cur = list[i];
			if (cur.GetType() != ScriptingValueType::Number) {
				THROW_EXCEPTION("Please supply list of numbers!")
			}
		}

		float* numbers = new float[list.size()];
		for (int i = 0; i < list.size(); ++i) {
			const Value& cur = list[i];
			numbers[i] = static_cast<float>(cur.GetNum());
		}

		std::string error;
		bool res = buffer->CopyData(numbers, list.size() * sizeof(float), error);
		delete[] numbers;

		if (!res) {
			THROW_EXCEPTION(error);
		}

		return Value();
	});

	Value& setStride = GetOrCreateProperty(nativeObject, "setStride");
	setStride = CreateNativeMethod(nativeObject, 1, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXBuffer* buffer = static_cast<DXBuffer*>(NativeObject::ExtractNativeObject(selfValue));

		Value strideValue = scope.GetProperty("param0");
		if (strideValue.GetType() != ScriptingValueType::Number) {
			THROW_EXCEPTION("Please supply buffer stride!");
		}

		int stride = static_cast<int>(strideValue.GetNum());
		if (stride < 0 || buffer->m_width % stride != 0) {
			THROW_EXCEPTION("Please supply a valid buffer stride!");
		}

		buffer->m_stride = stride;
		return Value();
	});

#undef THROW_EXCEPTION
}

bool rendering::DXBuffer::Place(ID3D12Device* device, ID3D12Heap* heap, UINT64 heapOffset, UINT64 width, std::string& errorMessage)
{
	CD3DX12_RESOURCE_DESC bufferDescription = CD3DX12_RESOURCE_DESC::Buffer(width);

	THROW_ERROR(
		device->CreatePlacedResource(heap, heapOffset, &bufferDescription, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_buffer)),
		"Can't place buffer in the heap!")

	return true;
}

bool rendering::DXBuffer::CopyData(void* data, int dataSize, std::string& errorMessage)
{
	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);

	void* dst = nullptr;

	THROW_ERROR(
		m_buffer->Map(0, &readRange, &dst),
		"Can't map Vertex Buffer!")

	memcpy(dst, data, dataSize);
	m_buffer->Unmap(0, nullptr);

	return true;
}

ID3D12Resource* rendering::DXBuffer::GetBuffer() const
{
	return m_buffer.Get();
}

UINT rendering::DXBuffer::GetBufferWidth() const
{
	return m_width;
}

UINT rendering::DXBuffer::GetStride() const
{
	return m_stride;
}

UINT rendering::DXBuffer::GetElementCount() const
{
	return m_width / m_stride;
}

#undef THROW_ERROR