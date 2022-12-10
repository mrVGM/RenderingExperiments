#include "helper/dxUpdater.h"

#include "nativeFunc.h"
#include "dxBuffer.h"
#include "d3dx12.h"
#include "dxRenderer.h"
#include "api.h"
#include "dataLib.h"

#include <sstream>
#include <list>

void rendering::helper::DXUpdater::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

	Value& p_settingsBuffer = GetOrCreateProperty(nativeObject, "settingsBuffer");

	Value& setSettingsBuffer = GetOrCreateProperty(nativeObject, "setSettingsBuffer");
	setSettingsBuffer = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXUpdater* self = static_cast<DXUpdater*>(NativeObject::ExtractNativeObject(selfValue));

		Value bufferValue = scope.GetProperty("param0");
		DXBuffer* buffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(bufferValue));
		if (!buffer) {
			THROW_EXCEPTION("Please supply settings buffer!")
		}

		p_settingsBuffer = bufferValue;
		self->m_settingsBuffer = buffer->GetBuffer();

		return Value();
	});

#undef THROW_EXCEPTION
}

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

void rendering::helper::DXUpdater::Update(double dt)
{
	using namespace interpreter;
	m_timeSinceUpdate += dt;
	if (m_timeSinceUpdate < m_updateTime) {
		return;
	}

	m_timeSinceUpdate = 0;

	Value api = GetAPI();
	Value appContext = api.GetProperty("app_context");
	
	std::string rootDir = appContext.GetProperty("root_dir").GetString();
	std::stringstream ss(data::GetLibrary().ReadFileByPath(rootDir + "clouds/settings.txt"));

	std::string dummy;
	float val;

	CD3DX12_RANGE readRange(0, 0);

	void* dst = nullptr;

	m_settingsBuffer->Map(0, &readRange, &dst);
	float* floatData = reinterpret_cast<float*>(dst);
	while (ss >> dummy) {
		ss >> val;
		*floatData = val;
		floatData += 1;
	}
	m_settingsBuffer->Unmap(0, nullptr);
}

#undef THROW_ERROR
