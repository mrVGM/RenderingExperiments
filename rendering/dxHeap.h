#pragma once

#include "nativeObject.h"
#include "dxDevice.h"

#include <d3d12.h>
#include <wrl.h>
#include <string>

namespace rendering
{
	class DXHeap : public interpreter::INativeObject
	{
		virtual void InitProperties(interpreter::NativeObject& nativeObject);

		Microsoft::WRL::ComPtr<ID3D12Heap> m_heap;
		bool Init(DXDevice& device, const D3D12_HEAP_DESC& desc, std::string errorMessage);
	public:

	};
}