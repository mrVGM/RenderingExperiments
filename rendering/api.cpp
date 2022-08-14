#include "api.h"

#include "utils.h"
#include "nativeFunc.h"
#include "nativeObject.h"

#include "window.h"
#include "dxDevice.h"

namespace rendering
{
	interpreter::Value m_api;

	void Create()
	{
		m_api = interpreter::utils::GetEmptyObject();

		m_api.SetProperty("window", interpreter::CreateNativeFunc(0, [](interpreter::Value scope) {
			Window* wnd = new Window();
			return interpreter::NativeObject::Create(wnd);
		}));

		m_api.SetProperty("device", interpreter::CreateNativeFunc(0, [](interpreter::Value scope) {
			DXDevice* device = new DXDevice();
			return interpreter::NativeObject::Create(device);
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