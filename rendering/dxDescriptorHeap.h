#pragma once

#include "nativeObject.h"

#include <d3d12.h>
#include <wrl.h>
#include <list>

namespace rendering
{
	class DXDescriptorHeap : public interpreter::INativeObject
	{
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;

		bool Create(
			ID3D12Device* device,
			const std::list<interpreter::Value>& buffers,
			std::string& errorMessage);

		void InitProperties(interpreter::NativeObject& nativeObject) override;
	public:
		ID3D12DescriptorHeap* GetHeap() const;
	};
}