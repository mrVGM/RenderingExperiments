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

	Value& heapProp = GetOrCreateProperty(nativeObject, "heap");

	Value& init = GetOrCreateProperty(nativeObject, "init");
	init = CreateNativeMethod(nativeObject, 3, [](Value scope) {
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

		Value allowUAValue = scope.GetProperty("param2");
		if (allowUAValue.GetType() != ScriptingValueType::Number) {
			THROW_EXCEPTION("Please supply UA value!");
		}

		texture->m_width = width;
		texture->m_height = height;
		texture->m_allowUA = static_cast<bool>(allowUAValue.GetNum());
		texture->m_format = DXGI_FORMAT_R32G32B32A32_FLOAT;

		return Value();
	});

	Value& init3D = GetOrCreateProperty(nativeObject, "init3D");
	init3D = CreateNativeMethod(nativeObject, 4, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXTexture* texture = static_cast<DXTexture*>(NativeObject::ExtractNativeObject(selfValue));

		texture->m_dimension = 3;

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

		Value depthValue = scope.GetProperty("param2");
		if (depthValue.GetType() != ScriptingValueType::Number) {
			THROW_EXCEPTION("Please supply texture depth!");
		}
		int depth = depthValue.GetNum();
		if (depth <= 0) {
			THROW_EXCEPTION("Please supply a valid texture depth!");
		}

		Value allowUAValue = scope.GetProperty("param3");
		if (allowUAValue.GetType() != ScriptingValueType::Number) {
			THROW_EXCEPTION("Please supply UA value!");
		}

		texture->m_width = width;
		texture->m_height = height;
		texture->m_depth = depth;
		texture->m_allowUA = static_cast<bool>(allowUAValue.GetNum());
		texture->m_format = DXGI_FORMAT_R32G32B32A32_FLOAT;

		return Value();
	});

	Value& initAsRT = GetOrCreateProperty(nativeObject, "initAsRT");
	initAsRT = CreateNativeMethod(nativeObject, 2, [](Value scope) {
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
		texture->m_allowUA = false;
		texture->m_format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		texture->m_renderTarget = true;

		return Value();
	});

	Value& initAsDS = GetOrCreateProperty(nativeObject, "initAsDS");
	initAsDS = CreateNativeMethod(nativeObject, 2, [](Value scope) {
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
		texture->m_allowUA = false;
		texture->m_format = DXGI_FORMAT_D32_FLOAT;
		texture->m_depthStencil = true;

		return Value();
	});

	Value& place = GetOrCreateProperty(nativeObject, "place");
	place = CreateNativeMethod(nativeObject, 3, [&](Value scope) {
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

		D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT;
		std::string heapType = heap->GetHeapType();
		if (heapType == "UPLOAD") {
			initialResourceState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ;
		}
		if (heapType == "READBACK") {
			initialResourceState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST;
		}
		if (texture->m_depthStencil) {
			initialResourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		}

		std::string error;
		bool res = texture->Place(
			&device->GetDevice(),
			heap->GetHeap(),
			offset,
			initialResourceState,
			error);

		if (!res) {
			THROW_EXCEPTION(error);
		}

		heapProp = heapValue;

		return Value();
	});

	Value& getAllocationSize = GetOrCreateProperty(nativeObject, "getAllocationSize");
	getAllocationSize = CreateNativeMethod(nativeObject, 1, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXTexture* texture = static_cast<DXTexture*>(NativeObject::ExtractNativeObject(selfValue));

		Value deviceValue = scope.GetProperty("param0");
		DXDevice* device = dynamic_cast<DXDevice*>(NativeObject::ExtractNativeObject(deviceValue));

		if (!device) {
			THROW_EXCEPTION("Please supply device!");
		}

		D3D12_RESOURCE_ALLOCATION_INFO info = texture->GetTextureAllocationInfo(&device->GetDevice());

		return Value(info.SizeInBytes);
	});

#undef THROW_EXCEPTION
}

bool rendering::DXTexture::Place(
	ID3D12Device* device,
	ID3D12Heap* heap,
	UINT64 heapOffset,
	D3D12_RESOURCE_STATES initialState,
	std::string& errorMessage)
{
	D3D12_RESOURCE_DESC textureDesc = GetTextureDescription();
	
	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	THROW_ERROR(
		device->CreatePlacedResource(
			heap,
			heapOffset,
			&textureDesc,
			initialState,
			m_depthStencil ? &depthOptimizedClearValue : nullptr,
			IID_PPV_ARGS(&m_texture)),
		"Can't place texture in the heap!")

	return true;
}

ID3D12Resource* rendering::DXTexture::GetTexture() const
{
	return m_texture.Get();
}

D3D12_RESOURCE_DESC rendering::DXTexture::GetTextureDescription() const
{
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	if (m_allowUA) {
		flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}
	if (m_renderTarget) {
		flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}
	if (m_depthStencil) {
		flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}

	CD3DX12_RESOURCE_DESC textureDesc = {};

	if (m_dimension == 2) {
		textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			m_format,
			m_width,
			m_height,
			1,
			0,
			1,
			0,
			flags);
	}

	if (m_dimension == 3) {
		textureDesc = CD3DX12_RESOURCE_DESC::Tex3D(
			m_format,
			m_width,
			m_height,
			m_depth,
			0,
			flags);
	}

	return textureDesc;
}

DXGI_FORMAT rendering::DXTexture::GetFormat() const
{
	return m_format;
}

D3D12_RESOURCE_ALLOCATION_INFO rendering::DXTexture::GetTextureAllocationInfo(ID3D12Device* device)
{
	D3D12_RESOURCE_DESC textureDesc = GetTextureDescription();
	D3D12_RESOURCE_ALLOCATION_INFO info = device->GetResourceAllocationInfo(0, 1, &textureDesc);
	return info;
}

UINT rendering::DXTexture::GetTextureWidth() const
{
	return m_width;
}

UINT rendering::DXTexture::GetTextureHeight() const
{
	return m_height;
}

UINT rendering::DXTexture::GetTextureDimension() const
{
	return m_dimension;
}

UINT rendering::DXTexture::GetTextureDepth() const
{
	return m_depth;
}

UINT rendering::DXTexture::GetTexturePixelSize() const
{
	return 16;
}

#undef THROW_ERROR