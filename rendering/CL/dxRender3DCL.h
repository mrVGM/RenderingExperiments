#pragma once

#include "nativeObject.h"

#include "d3dx12.h"

#include <d3d12.h>
#include <wrl.h>
#include <string>

namespace rendering
{
	class DXRender3DCL : public interpreter::INativeObject
	{
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		bool Create(
			ID3D12Device* device,
			ID3DBlob* vertexShader,
			ID3DBlob* pixelShader,
			std::string& errorMessage);
		bool Populate(
			const CD3DX12_VIEWPORT* viewport,
			CD3DX12_RECT* scissorRect,
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle,
			ID3D12Resource* renderTarget,
			const D3D12_VERTEX_BUFFER_VIEW* vertexBufferView,
			ID3D12DescriptorHeap* descHeap,
			ID3D12Resource* constantBuffer,
			std::string& errorMessage);

		bool Execute(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, int signal, std::string& error);

	public:
	};
}