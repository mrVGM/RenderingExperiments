#pragma once

#include "nativeObject.h"

#include <d3d12.h>
#include <wrl.h>
#include <string>

namespace rendering
{
	class DXBuffer : public interpreter::INativeObject
	{
		UINT m_width = -1;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_buffer;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		bool Place(ID3D12Device* device, ID3D12Heap* heap, UINT64 heapOffset, UINT64 width, std::string& errorMessage);
	};
}