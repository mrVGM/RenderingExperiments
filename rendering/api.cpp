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
#include "dxCopyCommandQueue.h"
#include "dxTexture.h"
#include "dxCopyCL.h"

#include "clouds/dxWorlyTextureComputeCL.h"
#include "CL/dxDisplayWorlyCL.h"

#include "deferred/dxLitMatCL.h"

#include "helper/dxCamera.h"

#include "primitives/cube.h"

#include "CL/dxPlainCL.h"

#include "deferred/gBuffer.h"
#include "deferred/dxLitPassCL.h"

#include "materials/dxCloudMatCL.h"
#include "materials/dxUnlitMatCL.h"

#include "clouds/cloudSettingsReader.h"
#include "clouds/dxCloudsCamera.h"

#include "CL/dxClearRTCL.h"

#include <cmath>
#include <numbers>

namespace rendering
{
	interpreter::Value m_api;

	void Create()
	{
		using namespace interpreter;

		m_api = interpreter::utils::GetEmptyObject();

		m_api.SetProperty("app_context", interpreter::utils::GetEmptyObject());
		Value app_context = m_api.GetProperty("app_context");

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

		m_api.SetProperty("copyCommandQueue", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXCopyCommandQueue* commandQueue = new DXCopyCommandQueue();
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
			clouds::DXWorlyTextureComputeCL* worlyTextureComputeCL = new clouds::DXWorlyTextureComputeCL();
			return NativeObject::Create(worlyTextureComputeCL);
		}));

		worly.SetProperty("displayWorly", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXDisplayWorlyCL* displayWorly = new DXDisplayWorlyCL();
			return NativeObject::Create(displayWorly);
		}));
#pragma endregion

#pragma region Math Functions
		m_api.SetProperty("math", interpreter::utils::GetEmptyObject());
		Value math = m_api.GetProperty("math");

		math.SetProperty("sqrt", interpreter::CreateNativeFunc(1, [](Value scope) {
			double x = scope.GetProperty("param0").GetNum();
			return Value(sqrt(x));
		}));

		math.SetProperty("sin", interpreter::CreateNativeFunc(1, [](Value scope) {
			double x = scope.GetProperty("param0").GetNum();
			return Value(sin(x));
		}));

		math.SetProperty("cos", interpreter::CreateNativeFunc(1, [](Value scope) {
			double x = scope.GetProperty("param0").GetNum();
			return Value(cos(x));
		}));

		math.SetProperty("asin", interpreter::CreateNativeFunc(1, [](Value scope) {
			double x = scope.GetProperty("param0").GetNum();
			return Value(asin(x));
		}));

		math.SetProperty("acos", interpreter::CreateNativeFunc(1, [](Value scope) {
			double x = scope.GetProperty("param0").GetNum();
			return Value(asin(x));
		}));

		math.SetProperty("PI", Value(std::numbers::pi));
#pragma endregion

#pragma region Primitives
		m_api.SetProperty("primitives", interpreter::utils::GetEmptyObject());
		Value primitives = m_api.GetProperty("primitives");

		primitives.SetProperty("cube", interpreter::CreateNativeFunc(0, [](Value scope) {
			rendering::primitives::Cube* cube = new rendering::primitives::Cube();
			return NativeObject::Create(cube);
		}));
#pragma endregion

#pragma region Deffered Shading

		m_api.SetProperty("deferred", interpreter::utils::GetEmptyObject());
		Value deferred = m_api.GetProperty("deferred");
		
		deferred.SetProperty("gBuffer", interpreter::CreateNativeFunc(0, [](Value scope) {
			rendering::deferred::GBuffer* gBuffer = new rendering::deferred::GBuffer();
			return NativeObject::Create(gBuffer);
		}));

		deferred.SetProperty("litPass", interpreter::CreateNativeFunc(0, [](Value scope) {
			rendering::deferred::DXLitPassCL* litPass = new rendering::deferred::DXLitPassCL();
			return NativeObject::Create(litPass);
		}));

		deferred.SetProperty("litMatCL", interpreter::CreateNativeFunc(0, [](Value scope) {
			deferred::DXLitMatCL* litMat = new deferred::DXLitMatCL();
			return NativeObject::Create(litMat);
		}));

		deferred.SetProperty("cloudMatCL", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXCloudMatCL* cloudMat = new DXCloudMatCL();
			return NativeObject::Create(cloudMat);
		}));

		deferred.SetProperty("unlitMatCL", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXUnlitMatCL* unlitMat = new DXUnlitMatCL();
			return NativeObject::Create(unlitMat);
		}));

#pragma endregion

#pragma region AUX
		m_api.SetProperty("aux", interpreter::utils::GetEmptyObject());
		Value aux = m_api.GetProperty("aux");

		aux.SetProperty("camera", CreateNativeFunc(0, [](Value scope) {
			DXCamera* camera = new DXCamera();
			return NativeObject::Create(camera);
		}));

		aux.SetProperty("plainCL", CreateNativeFunc(0, [](Value scope) {
			DXPlainCL* plainCL = new DXPlainCL();
			return NativeObject::Create(plainCL);
		}));

		aux.SetProperty("cloudSettingReader", CreateNativeFunc(0, [](Value scope) {
			clouds::CloudSettingsReader* settingsReader = new clouds::CloudSettingsReader();
			return NativeObject::Create(settingsReader);
		}));

		aux.SetProperty("clearRT", CreateNativeFunc(0, [](Value scope) {
			DXClearRTCL* clearRT = new DXClearRTCL();
			return NativeObject::Create(clearRT);
		}));
#pragma endregion

#pragma region Clouds
		m_api.SetProperty("clouds", interpreter::utils::GetEmptyObject());
		Value clouds = m_api.GetProperty("clouds");

		clouds.SetProperty("camera", CreateNativeFunc(0, [](Value scope) {
			clouds::DXCloudsCamera* camera = new clouds::DXCloudsCamera();
			return NativeObject::Create(camera);
		}));

		clouds.SetProperty("textureCompute", interpreter::CreateNativeFunc(0, [](Value scope) {
			clouds::DXWorlyTextureComputeCL* worlyTextureComputeCL = new clouds::DXWorlyTextureComputeCL();
			return NativeObject::Create(worlyTextureComputeCL);
		}));

		clouds.SetProperty("cloudSettingReader", CreateNativeFunc(0, [](Value scope) {
			clouds::CloudSettingsReader* settingsReader = new clouds::CloudSettingsReader();
			return NativeObject::Create(settingsReader);
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