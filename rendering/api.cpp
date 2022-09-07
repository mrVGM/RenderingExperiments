#include "api.h"

#include "utils.h"
#include "nativeFunc.h"
#include "nativeObject.h"

#include "window.h"
#include "dxDevice.h"
#include "dxVertexShader.h"
#include "dxPixelShader.h"
#include "dxHeap.h"
#include "dxFence.h"
#include "dxFenceEvent.h"
#include "dxBuffer.h"
#include "dxCommandQueue.h"
#include "dxSwapChain.h"
#include "dxDescriptorHeap.h"
#include "dxComputeShader.h"
#include "dxComputeCommandQueue.h"
#include "dxTexture.h"
#include "dxCopyCL.h"

#include "CL/dxWorlyTextureComputeCL.h"
#include "CL/dxDisplayWorlyCL.h"

namespace rendering
{
	interpreter::Value m_api;

	void Create()
	{
		using namespace interpreter;

		m_api = interpreter::utils::GetEmptyObject();

#pragma region Base Objects
		m_api.SetProperty("window", interpreter::CreateNativeFunc(0, [](Value scope) {
			Window* wnd = new Window();
			return NativeObject::Create(wnd);
		}));

		m_api.SetProperty("device", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXDevice* device = new DXDevice();
			return NativeObject::Create(device);
		}));

		m_api.SetProperty("swapChain", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXSwapChain* swapChain = new DXSwapChain();
			return NativeObject::Create(swapChain);
		}));

		m_api.SetProperty("commandQueue", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXCommandQueue* commandQuue = new DXCommandQueue();
			return NativeObject::Create(commandQuue);
		}));

		m_api.SetProperty("computeCommandQueue", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXComputeCommandQueue* commandQueue = new DXComputeCommandQueue();
			return NativeObject::Create(commandQueue);
		}));
#pragma endregion

#pragma region Sync Objects
		m_api.SetProperty("fence", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXFence* fence = new DXFence();
			return NativeObject::Create(fence);
		}));

		m_api.SetProperty("fenceEvent", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXFenceEvent* fenceEvent = new DXFenceEvent();
			return NativeObject::Create(fenceEvent);
		}));
#pragma endregion

#pragma region Resource Objects
		m_api.SetProperty("heap", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXHeap* heap = new DXHeap();
			return NativeObject::Create(heap);
		}));

		m_api.SetProperty("descriptorHeap", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXDescriptorHeap* descriptorHeap = new DXDescriptorHeap();
			return NativeObject::Create(descriptorHeap);
		}));
		
		m_api.SetProperty("buffer", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXBuffer* buffer = new DXBuffer();
			return NativeObject::Create(buffer);
		}));

		m_api.SetProperty("texture", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXTexture* texture = new DXTexture();
			return NativeObject::Create(texture);
		}));

		m_api.SetProperty("copyCL", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXCopyCL* copyCL = new DXCopyCL();
			return NativeObject::Create(copyCL);
		}));
#pragma endregion

#pragma region Shaders
		m_api.SetProperty("vertexShader", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXVertexShader* vertexShader = new DXVertexShader();
			return NativeObject::Create(vertexShader);
		}));

		m_api.SetProperty("pixelShader", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXPixelShader* pixelShader = new DXPixelShader();
			return NativeObject::Create(pixelShader);
		}));

		m_api.SetProperty("computeShader", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXComputeShader* computeShader = new DXComputeShader();
			return NativeObject::Create(computeShader);
		}));
#pragma endregion

#pragma region Worly Texture
		m_api.SetProperty("worly", interpreter::utils::GetEmptyObject());
		Value worly = m_api.GetProperty("worly");

		worly.SetProperty("textureCompute", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXWorlyTextureComputeCL* worlyTextureComputeCL = new DXWorlyTextureComputeCL();
			return NativeObject::Create(worlyTextureComputeCL);
		}));

		worly.SetProperty("displayWorly", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXDisplayWorlyCL* displayWorly = new DXDisplayWorlyCL();
			return NativeObject::Create(displayWorly);
		}));
#pragma endregion
	}
}

interpreter::Value rendering::GetAPI()
{
	if (m_api.IsNone()) {
		Create();
	}
	return m_api;
}