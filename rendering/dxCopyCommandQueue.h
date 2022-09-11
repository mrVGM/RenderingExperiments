#pragma once

#include "nativeObject.h"

#include <d3d12.h>
#include <wrl.h>
#include <string>

namespace rendering
{
	class DXCopyCommandQueue : public interpreter::INativeObject
	{
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;

		void InitProperties(interpreter::NativeObject& nativeObject) override;
		bool Create(ID3D12Device* device, std::string& errorMessage);
	public:

		ID3D12CommandQueue* GetCommandQueue();
	};
}