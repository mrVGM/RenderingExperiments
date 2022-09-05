#pragma once

#include "nativeObject.h"

#include "d3dx12.h"
#include "dxCommandQueue.h"
#include "dxFence.h"

#include <d3d12.h>
#include <wrl.h>
#include <string>

namespace rendering
{
	class DXCopyCL : public interpreter::INativeObject
	{
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		bool Create(
			ID3D12Device* device,
			std::string& errorMessage);

		bool Populate(
			ID3D12Resource* dst,
			ID3D12Resource* src,
			std::string& errorMessage);

		bool Execute(
			ID3D12CommandQueue* commandQueue,
			ID3D12Fence* fence,
			int signal,
			std::string& errorMessage);

	public:
		ID3D12GraphicsCommandList* GetCommandList() const;
	};
}