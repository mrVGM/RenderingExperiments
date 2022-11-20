#pragma once

#include "nativeObject.h"

#include "d3dx12.h"

#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <list>

namespace rendering::clouds
{
	class DXWorlyTextureComputeCL : public interpreter::INativeObject
	{
		enum ComputeRootParameters : UINT32
		{
			ComputeRootCBV = 0,
			ComputeRootUAVTable,
			ComputeRootSRVTable,
			ComputeRootParametersCount
		};

		struct NoiseSettings
		{
			int m_cells;
			int m_octaves;
			float m_persistance;
			float m_scale;
		};

		std::list<NoiseSettings> m_noiseSettings;

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
			ID3D12Resource* constantBuff,
			ID3D12Resource* tex,
			ID3D12DescriptorHeap* srvUavHeap,
			int threadGroupCountX,
			int threadGroupCountY,
			int threadGroupCountZ,
			std::string& errorMessage);

		bool ExecuteCompute(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, int signal, std::string& error);
		bool ExecutePrepareForPS(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, int signal, std::string& error);

		bool SetConstantBuffer(ID3D12Resource* buffer, int texSize, std::string& errorMessage);
		bool SetSRVBuffer(std::list<ID3D12Resource*> buffers, std::string& errorMessage);
		void GetSRVBufferSize(std::list<int>& sizes) const;
		int GetSRVBufferStride() const;
	public:

		DXWorlyTextureComputeCL();
	};
}