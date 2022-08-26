#pragma once

#include "nativeObject.h"

#include "dxBuffer.h"

#include <d3d12.h>
#include <wrl.h>
#include <vector>

namespace rendering
{
	class DXDescriptorHeap : public interpreter::INativeObject
	{
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;

		bool Create(
			ID3D12Device* device,
			const std::vector<DXBuffer*>& constantBuffers,
			std::string& errorMessage);

		void InitProperties(interpreter::NativeObject& nativeObject) override;
	public:
	};
}