#include "dxRenderer.h"

#include "nativeFunc.h"
#include "dxDevice.h"
#include "dxSwapChain.h"
#include "dxCommandQueue.h"
#include "dxTexture.h"
#include "dxFence.h"
#include "window.h"
#include "dxBuffer.h"
#include "scene/dxScene.h"
#include "scene/dxMeshRepo.h"
#include "materials/dxMaterialRepo.h"

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

void rendering::DXRenderer::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

	Value& p_wnd = GetOrCreateProperty(nativeObject, "window");

	Value& p_dsTexture = GetOrCreateProperty(nativeObject, "dsTexture");
	Value& p_camBuff = GetOrCreateProperty(nativeObject, "camBuff");

	Value& p_device = GetOrCreateProperty(nativeObject, "device");
	Value& p_swapChain = GetOrCreateProperty(nativeObject, "swapChain");
	Value& p_commandQueue = GetOrCreateProperty(nativeObject, "commandQueue");
	Value& p_renderStages = GetOrCreateProperty(nativeObject, "renderStages");
	Value& p_fence = GetOrCreateProperty(nativeObject, "fence");

	Value& p_scene = GetOrCreateProperty(nativeObject, "scene");
	Value& p_meshRepo = GetOrCreateProperty(nativeObject, "meshRepo");
	Value& p_materialRepo = GetOrCreateProperty(nativeObject, "materialRepo");
	
	Value& setMaterialRepo = GetOrCreateProperty(nativeObject, "setMaterialRepo");
	setMaterialRepo = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXRenderer* self = static_cast<DXRenderer*>(NativeObject::ExtractNativeObject(selfValue));

		Value materialRepoValue = scope.GetProperty("param0");
		material::DXMaterialRepo* materialRepo = dynamic_cast<material::DXMaterialRepo*>(NativeObject::ExtractNativeObject(materialRepoValue));

		if (!materialRepo) {
			THROW_EXCEPTION("Please supply a Material Repo!")
		}

		p_materialRepo = materialRepoValue;
		self->m_materialRepo = materialRepo;

		return Value();
	});

	Value& setScene = GetOrCreateProperty(nativeObject, "setScene");
	setScene = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXRenderer* self = static_cast<DXRenderer*>(NativeObject::ExtractNativeObject(selfValue));

		Value sceneValue = scope.GetProperty("param0");
		scene::DXScene* scene = dynamic_cast<scene::DXScene*>(NativeObject::ExtractNativeObject(sceneValue));

		if (!scene) {
			THROW_EXCEPTION("Please supply a Scene!")
		}

		p_scene = sceneValue;
		self->m_scene = scene;

		return Value();
	});

	Value& setMeshRepo = GetOrCreateProperty(nativeObject, "setMeshRepo");
	setMeshRepo = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXRenderer* self = static_cast<DXRenderer*>(NativeObject::ExtractNativeObject(selfValue));

		Value meshRepoValue = scope.GetProperty("param0");
		scene::DXMeshRepo* meshRepo = dynamic_cast<scene::DXMeshRepo*>(NativeObject::ExtractNativeObject(meshRepoValue));

		if (!meshRepo) {
			THROW_EXCEPTION("Please supply a Mesh Repo!")
		}

		p_meshRepo = meshRepoValue;
		self->m_meshRepo = meshRepo;

		return Value();
	});

	Value& setWindow = GetOrCreateProperty(nativeObject, "setWindow");
	setWindow = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXRenderer* self = static_cast<DXRenderer*>(NativeObject::ExtractNativeObject(selfValue));

		Value wndValue = scope.GetProperty("param0");
		Window* wnd = dynamic_cast<Window*>(NativeObject::ExtractNativeObject(wndValue));

		if (!wnd) {
			THROW_EXCEPTION("Please supply a window!")
		}

		p_wnd = wndValue;
		self->m_wnd = wnd;

		return Value();
	});

	Value& setDevice = GetOrCreateProperty(nativeObject, "setDevice");
	setDevice = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXRenderer* self = static_cast<DXRenderer*>(NativeObject::ExtractNativeObject(selfValue));

		Value deviceValue = scope.GetProperty("param0");
		DXDevice* device = dynamic_cast<DXDevice*>(NativeObject::ExtractNativeObject(deviceValue));

		if (!device) {
			THROW_EXCEPTION("Please supply a device!")
		}

		p_device = deviceValue;
		self->m_device = &device->GetDevice();

		return Value();
	});

	Value& setSwapChain = GetOrCreateProperty(nativeObject, "setSwapChain");
	setSwapChain = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXRenderer* self = static_cast<DXRenderer*>(NativeObject::ExtractNativeObject(selfValue));

		Value swapChainValue = scope.GetProperty("param0");
		DXSwapChain* swapChain = dynamic_cast<DXSwapChain*>(NativeObject::ExtractNativeObject(swapChainValue));

		if (!swapChain) {
			THROW_EXCEPTION("Please supply a SwapChain!")
		}

		p_swapChain = swapChainValue;
		self->m_swapChain = swapChain;

		return Value();
	});

	Value& setCommandQueue = GetOrCreateProperty(nativeObject, "setCommandQueue");
	setCommandQueue = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXRenderer* self = static_cast<DXRenderer*>(NativeObject::ExtractNativeObject(selfValue));

		Value commandQueueValue = scope.GetProperty("param0");
		DXCommandQueue* commandQueue = dynamic_cast<DXCommandQueue*>(NativeObject::ExtractNativeObject(commandQueueValue));

		if (!commandQueue) {
			THROW_EXCEPTION("Please supply a Command Queue!")
		}

		p_commandQueue = commandQueueValue;
		self->m_commandQueue = commandQueue->GetCommandQueue();

		return Value();
	});
	
	Value& setDSV = GetOrCreateProperty(nativeObject, "setDSV");
	setDSV = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXRenderer* self = static_cast<DXRenderer*>(NativeObject::ExtractNativeObject(selfValue));

		Value dsTexValue = scope.GetProperty("param0");
		DXTexture* dsTex = dynamic_cast<DXTexture*>(NativeObject::ExtractNativeObject(dsTexValue));

		if (!dsTex) {
			THROW_EXCEPTION("Please supply a DS Texture!")
		}

		p_dsTexture = dsTexValue;

		std::string error;
		bool res = self->SetupDSVHeap(dsTex->GetTexture(), error);
		if (!res) {
			THROW_EXCEPTION(error)
		}

		return Value();
	});

	Value& setCamBuff = GetOrCreateProperty(nativeObject, "setCamBuff");
	setCamBuff = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXRenderer* self = static_cast<DXRenderer*>(NativeObject::ExtractNativeObject(selfValue));

		Value camBuffValue = scope.GetProperty("param0");
		DXBuffer* camBuff = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(camBuffValue));

		if (!camBuff) {
			THROW_EXCEPTION("Please supply a camera buffer!")
		}

		p_camBuff = camBuffValue;
		self->m_camBuffer = camBuff->GetBuffer();

		return Value();
	});


	Value& addRenderStage = GetOrCreateProperty(nativeObject, "addRenderStage");
	addRenderStage = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXRenderer* self = static_cast<DXRenderer*>(NativeObject::ExtractNativeObject(selfValue));

		Value renderStageValue = scope.GetProperty("param0");
		IRenderStage* renderStage = dynamic_cast<IRenderStage*>(NativeObject::ExtractNativeObject(renderStageValue));

		if (!renderStage) {
			THROW_EXCEPTION("Please supply a Render Stage!")
		}

		std::list<Value> tmp;
		if (!p_renderStages.IsNone()) {
			p_renderStages.ToList(tmp);
		}
		tmp.push_back(renderStageValue);
		p_renderStages = Value::FromList(tmp);

		self->m_renderStages.push_back(renderStage);

		return Value();
	});


	Value& initRenderStages = GetOrCreateProperty(nativeObject, "initRenderStages");
	initRenderStages = CreateNativeMethod(nativeObject, 0, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXRenderer* self = static_cast<DXRenderer*>(NativeObject::ExtractNativeObject(selfValue));

		for (std::list<IRenderStage*>::iterator it = self->m_renderStages.begin(); it != self->m_renderStages.end(); ++it) {
			IRenderStage* cur = *it;

			std::string error;
			bool res = cur->Init(*self, error);
			if (!res) {
				THROW_EXCEPTION("Can't Init Render Stages!")
			}
		}

		return Value();
	});

	Value& setFence = GetOrCreateProperty(nativeObject, "setFence");
	setFence = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXRenderer* self = static_cast<DXRenderer*>(NativeObject::ExtractNativeObject(selfValue));

		Value fenceValue = scope.GetProperty("param0");
		DXFence* fence = dynamic_cast<DXFence*>(NativeObject::ExtractNativeObject(fenceValue));

		if (!fence) {
			THROW_EXCEPTION("Please supply a fence!")
		}

		p_fence = fenceValue;
		self->m_fence = fence->GetFence();

		return Value();
	});


#undef THROW_EXCEPTION
}

bool rendering::DXRenderer::SetupDSVHeap(ID3D12Resource* dsTexture, std::string& errorMessage)
{
	{
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		THROW_ERROR(
			m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)),
			"Can't Create DSV Heap!"
		);
	}

	m_dsTexture = dsTexture;
	return true;
}

ID3D12Device* rendering::DXRenderer::GetDevice()
{
	return m_device;
}

rendering::ISwapChain* rendering::DXRenderer::GetISwapChain()
{
	return m_swapChain;
}

ID3D12CommandQueue* rendering::DXRenderer::GetCommandQueue()
{
	return m_commandQueue;
}

ID3D12Resource* rendering::DXRenderer::GetCamBuff()
{
	return m_camBuffer;
}

rendering::scene::IScene* rendering::DXRenderer::GetScene()
{
	return m_scene;
}

rendering::scene::IMeshRepo* rendering::DXRenderer::GetMeshRepo()
{
	return m_meshRepo;
}

rendering::material::IMaterialRepo* rendering::DXRenderer::GetMaterialRepo()
{
	return m_materialRepo;
}

bool rendering::DXRenderer::Render(std::string& errorMessage)
{
	m_swapChain->UpdateCurrentFrameIndex();

	for (std::list<IRenderStage*>::iterator it = m_renderStages.begin(); it != m_renderStages.end(); ++it) {
		IRenderStage* cur = *it;
		
		bool res = cur->Execute(*this, errorMessage);
		if (!res) {
			return false;
		}
	}

	bool res = m_swapChain->Present(errorMessage);
	if (!res) {
		return false;
	}

	m_commandQueue->Signal(m_fence, m_counter);
	return true;
}

bool rendering::DXRenderer::Wait(std::string& errorMessage)
{
	HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (!event) {
		errorMessage = "Can't create fence event!";
		return false;
	}

	HRESULT hr = m_fence->SetEventOnCompletion(m_counter, event);

	if (FAILED(hr)) {
		errorMessage = "Can't attach wait event to the fence!";
		return false;
	}

	WaitForSingleObject(event, INFINITE);
	++m_counter;
	return true;
}

#undef THROW_ERROR