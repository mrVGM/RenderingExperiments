#pragma once

#include "nativeObject.h"

#include "d3dx12.h"

#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <list>

namespace rendering::compute
{
	class DXNoiseTexture : public interpreter::INativeObject
	{
		enum ComputeRootParameters : UINT32
		{
			ComputeRootCBV = 0,
			ComputeRootUAVTable,
			ComputeRootSRVTable,
			ComputeRootParametersCount
		};

		ID3D12Resource* m_constantBuffer;
		ID3D12Resource* m_dataBuffer;
		ID3D12Resource* m_texture;
		ID3D12DescriptorHeap* m_descriptorHeap;

		int m_worly1Size = -1;
		int m_worly2Size = -1;
		int m_worly3Size = -1;

		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_computeAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_computeCL;

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_prepareForPixelAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_prepareForPixelCL;

		UINT m_srvUavDescriptorSize = 0;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		bool Create(
			ID3D12Device* device,
			ID3DBlob* computeShader,
			std::string& errorMessage);
		bool Populate(
			int threadGroupCountX,
			int threadGroupCountY,
			int threadGroupCountZ,
			std::string& errorMessage);

		bool ExecuteCompute(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, int signal, std::string& error);
		bool ExecutePrepareForPS(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, int signal, std::string& error);

		bool SetupDataBuffer(std::string& errorMessage);
	public:
	};
}