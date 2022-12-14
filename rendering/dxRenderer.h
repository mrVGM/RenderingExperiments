#pragma once

#include "nativeObject.h"
#include "d3dx12.h"
#include "IRenderStage.h"
#include "window.h"
#include "scene/IScene.h"
#include "scene/dxMeshRepo.h"
#include "materials/IMaterialRepo.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <list>
#include <string>

namespace rendering
{
	struct ISwapChain
	{
		CD3DX12_VIEWPORT m_viewport;
		CD3DX12_RECT m_scissorRect;
		virtual CD3DX12_CPU_DESCRIPTOR_HANDLE GetCurrentRTVDescriptor() const = 0;
		virtual ID3D12Resource* GetCurrentRenderTarget() const = 0;
		virtual void UpdateCurrentFrameIndex() = 0;
		virtual bool Present(std::string& errorMessage) = 0;
	};

	class DXRenderer : public interpreter::INativeObject
	{
		int m_counter = 0;
		Window* m_wnd = nullptr;
		ID3D12Device* m_device = nullptr;
		ISwapChain* m_swapChain = nullptr;
		ID3D12CommandQueue* m_commandQueue = nullptr;
		ID3D12Fence* m_fence = nullptr;

		ID3D12Resource* m_camBuffer = nullptr;

		ID3D12Resource* m_dsTexture = nullptr;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		bool SetupDSVHeap(ID3D12Resource* dsTexture, std::string& errorMessage);
		
		scene::IScene* m_scene = nullptr;
		scene::IMeshRepo* m_meshRepo = nullptr;
		material::IMaterialRepo* m_materialRepo = nullptr;

		std::list<IRenderStage*> m_renderStages;

	public:
		ID3D12Device* GetDevice();
		ISwapChain* GetISwapChain();
		ID3D12CommandQueue* GetCommandQueue();
		ID3D12Resource* GetCamBuff();
		scene::IScene* GetScene();
		scene::IMeshRepo* GetMeshRepo();
		material::IMaterialRepo* GetMaterialRepo();
		D3D12_CPU_DESCRIPTOR_HANDLE GetDSHandle();

		bool Render(std::string& errorMessage);
		bool Wait(std::string& errorMessage);
	};
}