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

namespace rendering
{
	interpreter::Value m_api;

	void Create()
	{
		using namespace interpreter;

		m_api = interpreter::utils::GetEmptyObject();

		m_api.SetProperty("window", interpreter::CreateNativeFunc(0, [](Value scope) {
			Window* wnd = new Window();
			return NativeObject::Create(wnd);
		}));

		m_api.SetProperty("device", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXDevice* device = new DXDevice();
			return NativeObject::Create(device);
		}));

		m_api.SetProperty("vertexShader", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXVertexShader* vertexShader = new DXVertexShader();
			return NativeObject::Create(vertexShader);
		}));

		m_api.SetProperty("pixelShader", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXPixelShader* pixelShader = new DXPixelShader();
			return NativeObject::Create(pixelShader);
		}));

		m_api.SetProperty("heap", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXHeap* heap = new DXHeap();
			return NativeObject::Create(heap);
		}));

		m_api.SetProperty("fence", interpreter::CreateNativeFunc(0, [](Value scope) {
			DXFence* fence = new DXFence();
			return NativeObject::Create(fence);
		}));
	}
}

interpreter::Value rendering::GetAPI()
{
	if (m_api.IsNone()) {
		Create();
	}
	return m_api;
}