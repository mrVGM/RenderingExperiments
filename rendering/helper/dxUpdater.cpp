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
	m_totalTime += dt;
	if (m_timeSinceUpdate < m_updateTime) {
		return;
	}

	m_timeSinceUpdate = 0;

	Value api = GetAPI();
	Value appContext = api.GetProperty("app_context");

	Value rendererValue = appContext.GetProperty("renderer");
	DXRenderer* renderer = nullptr;

	if (!rendererValue.IsNone()) {
		renderer = static_cast<DXRenderer*>(NativeObject::ExtractNativeObject(rendererValue));
	}
	
	std::string rootDir = appContext.GetProperty("root_dir").GetString();
	std::stringstream ss(data::GetLibrary().ReadFileByPath(rootDir + "clouds/settings.txt"));

	std::string dummy;
	float val;

	CD3DX12_RANGE readRange(0, 0);

	void* dst = nullptr;

	m_settingsBuffer->Map(0, &readRange, &dst);
	float* floatData = reinterpret_cast<float*>(dst);

	ss >> dummy;

	float sunPos[3];
	ss >> sunPos[0] >> sunPos[1] >> sunPos[2];

	sunPos[0] = 0 + 5 * cos(0.3 * m_totalTime);
	sunPos[1] = 5 + 5 * sin(0.3 * m_totalTime);
	sunPos[2] = 5 + 0;

	floatData[0] = sunPos[0];
	floatData[1] = sunPos[1];
	floatData[2] = sunPos[2];
	floatData += 3;

	if (renderer) {
		scene::IScene* scene = renderer->GetScene();
		std::map<std::string, scene::Object3D>::iterator sunIt = scene->m_objects.find("sun");

		if (sunIt != scene->m_objects.end()) {
			scene::Object3D& sunObj = sunIt->second;

			sunObj.m_transform.m_position[0] = sunPos[0];
			sunObj.m_transform.m_position[1] = sunPos[1];
			sunObj.m_transform.m_position[2] = sunPos[2];
		}
	}

	float lightIntensity;
	ss >> lightIntensity;

	*floatData = lightIntensity;
	floatData += 1;

	while (ss >> dummy) {
		ss >> val;
		*floatData = val;
		floatData += 1;
	}
	m_settingsBuffer->Unmap(0, nullptr);
}

#undef THROW_ERROR
