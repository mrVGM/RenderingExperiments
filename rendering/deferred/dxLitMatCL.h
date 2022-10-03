#pragma once

#include "nativeObject.h"

#include "d3dx12.h"

#include <d3d12.h>
#include <wrl.h>
#include <string>

namespace rendering::deferred
{
	class DXLitMatCL : public interpreter::INativeObject
	{
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

		ID3D12Resource* m_constantBuffer = nullptr;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		bool Create(
			ID3D12Device* device,
			ID3DBlob* vertexShader,
			ID3DBlob* pixelShader,
			ID3D12Resource* constantBuffer,
			std::string& errorMessage);
		bool Populate(
			const CD3DX12_VIEWPORT* viewport,
			CD3DX12_RECT* scissorRect,
			D3D12_CPU_DESCRIPTOR_HANDLE diffuseTexHandle,
			D3D12_CPU_DESCRIPTOR_HANDLE normalTexHandle,
			D3D12_CPU_DESCRIPTOR_HANDLE positionTexHandle,
			D3D12_CPU_DESCRIPTOR_HANDLE specularTexHandle,
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle,
			ID3D12Resource* vertexBuffer,
			int vertexBufferSize,
			int vertexBufferStride,
			ID3D12Resource* indexBuffer,
			int indexBufferSize,
			ID3D12Resource* instanceBuffer,
			int instanceBufferSize,
			int instanceBufferStride,
			std::string& errorMessage);

		bool Execute(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, int signal, std::string& error);
		bool ExecuteAsync(ID3D12CommandQueue* commandQueue, std::string& error);

	public:
	};
}