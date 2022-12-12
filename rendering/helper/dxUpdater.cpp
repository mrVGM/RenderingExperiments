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

void rendering::helper::DXUpdater::UpdateSettings()
{
	using namespace interpreter;

	Value api = GetAPI();
	Value appContext = api.GetProperty("app_context");
	std::string rootDir = appContext.GetProperty("root_dir").GetString();
	std::stringstream ss(data::GetLibrary().ReadFileByPath(rootDir + "clouds/settings.txt"));

	std::string settingName;

	while (ss >> settingName) {
		for (std::list<Setting>::iterator it = m_settings.begin(); it != m_settings.end(); ++it) {
			Setting& cur = *it;

			if (cur.m_name == settingName) {
				for (int i = 0; i < cur.m_dim; ++i) {
					ss >> cur.m_value[i];
				}
				break;
			}
		}
	}
}

void rendering::helper::DXUpdater::Update(double dt)
{
	using namespace interpreter;
	m_timeSinceUpdate += dt;
	m_totalTime += dt;

	if (m_timeSinceUpdate > m_updateTime) {
		UpdateSettings();
		m_timeSinceUpdate = 0;
	}

	for (std::list<Setting>::iterator it = m_settings.begin(); it != m_settings.end(); ++it) {
		Setting& cur = *it;

		if (cur.m_name == "m_lightPosition") {
			cur.m_value[0] = 0 + 2 * cos(0.1 * m_totalTime);
			cur.m_value[1] = 5 + 2 * sin(0.1 * m_totalTime);
			cur.m_value[2] = 5;
			break;
		}
	}

	CD3DX12_RANGE readRange(0, 0);
	void* dst = nullptr;

	HRESULT hr = m_settingsBuffer->Map(0, &readRange, &dst);
	if (FAILED(hr)) {
		return;
	}
	float* floatData = reinterpret_cast<float*>(dst);

	for (std::list<Setting>::iterator it = m_settings.begin(); it != m_settings.end(); ++it) {
		Setting& cur = *it;

		for (int i = 0; i < cur.m_dim; ++i) {
			*floatData = cur.m_value[i];
			++floatData;
		}
	}

	m_settingsBuffer->Unmap(0, nullptr);

	Value api = GetAPI();
	Value appContext = api.GetProperty("app_context");
	Value rendererValue = appContext.GetProperty("renderer");
	DXRenderer* renderer = nullptr;

	if (!rendererValue.IsNone()) {
		renderer = static_cast<DXRenderer*>(NativeObject::ExtractNativeObject(rendererValue));
	}

	if (!renderer) {
		return;
	}

	scene::IScene* scene = renderer->GetScene();
	std::map<std::string, scene::Object3D>::iterator sunIt = scene->m_objects.find("sun");

	if (sunIt == scene->m_objects.end()) {
		return;
	}
	scene::Object3D& sun = sunIt->second;

	for (std::list<Setting>::const_iterator it = m_settings.begin(); it != m_settings.end(); ++it) {
		const Setting& cur = *it;

		if (cur.m_name == "m_lightPosition") {
			sun.m_transform.m_position[0] = cur.m_value[0];
			sun.m_transform.m_position[1] = cur.m_value[1];
			sun.m_transform.m_position[2] = cur.m_value[2];

			break;
		}
	}
}

rendering::helper::DXUpdater::DXUpdater()
{
	m_settings.push_back(Setting{ "m_lightPosition", 4, {5, 5, 5, 1} });

	m_settings.push_back(Setting{ "m_sampleSteps", 1, 15 });
	m_settings.push_back(Setting{ "m_cloudAbsorbtion", 1, 6 });
	m_settings.push_back(Setting{ "m_densityOffset", 1, 3 });
	
	m_settings.push_back(Setting{ "m_cloudLightAbsorbtion", 1, 6 });
	m_settings.push_back(Setting{ "m_airLightAbsorbtion", 1, 0.2 });
	m_settings.push_back(Setting{ "m_g", 1, 0.8 });

	m_settings.push_back(Setting{ "m_worly1Weight", 1, 0.625 });
	m_settings.push_back(Setting{ "m_worly2Weight", 1, 0.25 });
	m_settings.push_back(Setting{ "m_worly3Weight", 1, 0.125 });
}

#undef THROW_ERROR
