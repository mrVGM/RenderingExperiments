#include "dxTexture.h"

#include "d3dx12.h"
#include "nativeFunc.h"
#include "dxDevice.h"
#include "dxHeap.h"

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

void rendering::DXTexture::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

	Value& init = GetOrCreateProperty(nativeObject, "init");
	init = CreateNativeMethod(nativeObject, 2, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXTexture* texture = static_cast<DXTexture*>(NativeObject::ExtractNativeObject(selfValue));

		Value widthValue = scope.GetProperty("param0");
		if (widthValue.GetType() != ScriptingValueType::Number) {
			THROW_EXCEPTION("Please supply texture width!");
		}
		int width = widthValue.GetNum();
		if (width <= 0) {
			THROW_EXCEPTION("Please supply a valid texture width!");
		}

		Value heightValue = scope.GetProperty("param1");
		if (heightValue.GetType() != ScriptingValueType::Number) {
			THROW_EXCEPTION("Please supply texture width!");
		}
		int height = heightValue.GetNum();
		if (height <= 0) {
			THROW_EXCEPTION("Please supply a valid texture height!");
		}

		texture->m_width = width;
		texture->m_height = height;
		texture->m_format = DXGI_FORMAT_R8G8B8A8_UNORM;

		return Value();
	});

	Value& place = GetOrCreateProperty(nativeObject, "place");
	place = CreateNativeMethod(nativeObject, 4, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXTexture* texture = static_cast<DXTexture*>(NativeObject::ExtractNativeObject(selfValue));

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

		Value allowUAValue = scope.GetProperty("param3");
		if (allowUAValue.GetType() != ScriptingValueType::Number) {
			THROW_EXCEPTION("Please supply allow UA value!");
		}

		std::string error;
		bool res = texture->Place(
			&device->GetDevice(),
			heap->GetHeap(),
			offset,
			static_cast<bool>(allowUAValue.GetNum()),
			error);

		if (!res) {
			THROW_EXCEPTION(error);
		}

		return Value();
	});

#undef THROW_EXCEPTION
}

bool rendering::DXTexture::Place(
	ID3D12Device* device,
	ID3D12Heap* heap,
	UINT64 heapOffset,
	bool allowUA,
	std::string& errorMessage)
{
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	if (allowUA) {
		flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.MipLevels = 1;
	textureDesc.Format = m_format;
	textureDesc.Width = m_width;
	textureDesc.Height = m_height;
	textureDesc.Flags = flags;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	THROW_ERROR(
		device->CreatePlacedResource(heap, heapOffset, &textureDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_texture)),
		"Can't place buffer in the heap!")

	return true;
}

ID3D12Resource* rendering::DXTexture::GetTexture() const
{
	return m_texture.Get();
}

DXGI_FORMAT rendering::DXTexture::GetFormat() const
{
	return m_format;
}

#undef THROW_ERROR