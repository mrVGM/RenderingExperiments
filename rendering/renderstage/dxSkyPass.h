#pragma once

#include "nativeObject.h"

#include "d3dx12.h"
#include "IRenderStage.h"

#include <d3d12.h>
#include <wrl.h>
#include <string>


namespace rendering::renderstage
{
	class DXSkyPass : public interpreter::INativeObject, public rendering::IRenderStage
	{
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