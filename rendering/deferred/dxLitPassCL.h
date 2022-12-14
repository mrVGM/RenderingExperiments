#pragma once

#include "nativeObject.h"
#include "d3dx12.h"

#include <d3d12.h>
#include <wrl.h>

namespace rendering::deferred
{
	class DXLitPassCL : public interpreter::INativeObject
	{
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocatorStart;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandListStart;


		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocatorEnd;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandListEnd;

		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		bool SetupStartCL(
			ID3D12Device* device,
			D3D12_CPU_DESCRIPTOR_HANDLE diffuseHandle,
			D3D12_CPU_DESCRIPTOR_HANDLE normalHandle,
			D3D12_CPU_DESCRIPTOR_HANDLE positionHandle,
			D3D12_CPU_DESCRIPTOR_HANDLE specularHandle,
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle,
			ID3D12Resource* diffuseTex,
			ID3D12Resource* normalTexture,
			ID3D12Resource* positionTexture,
			ID3D12Resource* specularTexture,
			std::string& errorMessage);

		bool SetupEndCL(
			ID3D12Device* device,
			ID3DBlob* vertexShader,
			ID3DBlob* pixelShader,
			ID3D12Resource* vertexBuffer,
			int vertexBufferWidth,
			int vertexBufferStride,
			std::string& errorMessage);

		bool PopulateEnd(
			const CD3DX12_VIEWPORT* viewport,
			CD3DX12_RECT* scissorRect,
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle,
			ID3D12Resource* renderTarget,
			const D3D12_VERTEX_BUFFER_VIEW* vertexBufferView,
			ID3D12Resource* camBuffer,
			ID3D12Resource* lightsConstantBuffer,
			ID3D12Resource* lightsBuffer,
			ID3D12Resource* diffuseTex,
			ID3D12Resource* normalTex,
			ID3D12Resource* positionTex,
			ID3D12Resource* specularTex,
			ID3D12DescriptorHeap* descriptorHeap,
			std::string& errorMessage);

		bool ExecuteStart(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, int signal, std::string& error);
		bool ExecuteEnd(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, int signal, std::string& error);
	public:
	};
}