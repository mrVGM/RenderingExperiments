#pragma once

#include "nativeObject.h"

#include "IRenderStage.h"

#include "materials/IMaterial.h"
#include "d3dx12.h"

#include <d3d12.h>
#include <wrl.h>
#include <string>


namespace rendering::renderstage
{
	class DXRenderPass : public interpreter::INativeObject, public rendering::IRenderStage
	{
		rendering::material::MaterialType m_type = rendering::material::MaterialType::Unlit;

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandListStart;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandListEnd;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		bool RenderMaterials(DXRenderer& renderer, std::string& errorMessage);

	public:
		bool Init(DXRenderer& renderer, std::string& errorMessage) override;
		bool Execute(DXRenderer& renderer, std::string& errorMessage) override;
	};
}