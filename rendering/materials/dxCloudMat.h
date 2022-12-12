#pragma once

#include "nativeObject.h"
#include "materials/IMaterial.h"

#include "d3dx12.h"

#include <d3d12.h>
#include <wrl.h>
#include <string>

namespace rendering::material
{
	class DXCloudMat : public interpreter::INativeObject, public IMaterial
	{
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

		ID3D12Resource* m_curRT = nullptr;

		ID3D12Resource* m_constantBuffer = nullptr;
		ID3D12Resource* m_noiseTexture = nullptr;
		ID3D12DescriptorHeap* m_descriptorHeap = nullptr;

		ID3DBlob* m_vertexShader = nullptr;
		ID3DBlob* m_pixelShader = nullptr;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

	public:
		MaterialType GetMaterialType() const override;
		bool Render(
			rendering::DXRenderer* renderer,
			ID3D12Resource* vertexBuffer,
			int vertexBufferSize,
			int vertexBufferStride,

			ID3D12Resource* instanceBuffer,
			int instanceBufferSize,
			int instanceBufferStride,

			ID3D12Resource* indexBuffer,
			int indexBufferSize,
			std::string& errorMessage) override;

		bool Init(DXRenderer& renderer, std::string& errorMessage) override;
	};
}