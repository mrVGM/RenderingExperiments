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
	class DXRenderCanvasesCL : public interpreter::INativeObject
	{
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_clearRTCL;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_finishCL;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		bool Create(
			ID3D12Device* device,
			std::string& errorMessage);

		bool Populate(
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle,
			ID3D12Resource* renderTarget,
			std::string& errorMessage);

		bool Execute(
			ID3D12CommandQueue* commandQueue,
			ID3D12Fence* fence,
			int signal,
			const std::vector<ID3D12GraphicsCommandList*>& canvases,
			std::string& errorMessage);

	public:
		ID3D12GraphicsCommandList* GetCommandList() const;
	};
}