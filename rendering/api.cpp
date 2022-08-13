#include "api.h"

#include "utils.h"
#include "nativeFunc.h"
#include "nativeObject.h"

#include "window.h"

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
	}
}

interpreter::Value rendering::GetAPI()
{
	if (m_api.IsNone()) {
		Create();
	}
	return m_api;
}