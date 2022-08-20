#pragma once

#include "nativeObject.h"

#include "dxDevice.h"
#include "dxFence.h"

#include <d3d12.h>
#include <wrl.h>
#include <string>

namespace rendering
{
	class DXHeap : public interpreter::INativeObject
	{
		virtual void InitProperties(interpreter::NativeObject& nativeObject);

		ID3D12Device3* m_device3 = nullptr;
		bool m_resident = false;

		Microsoft::WRL::ComPtr<ID3D12Heap> m_heap;
		bool Init(DXDevice& device, const D3D12_HEAP_DESC& desc, std::string errorMessage);
		bool MakeResident(DXFence* fence, int signalValue, std::string& errorMessage);
		bool Evict(std::string& errorMessage);
	public:
		~DXHeap();
	};
}