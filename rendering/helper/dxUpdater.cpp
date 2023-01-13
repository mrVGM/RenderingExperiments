#include "helper/dxUpdater.h"

#include "nativeFunc.h"
#include "dxBuffer.h"
#include "d3dx12.h"
#include "dxRenderer.h"
#include "api.h"
#include "dataLib.h"
#include "json.hpp"
#include "helper/dxCamera.h"

#include <sstream>
#include <list>
#include <thread>
#include <Windows.h>

namespace
{
	std::thread* m_thread = nullptr;
	rendering::helper::DXUpdater* m_updater = nullptr;

	void run() {
		rendering::helper::DXUpdater* updater = m_updater;
		m_updater = nullptr;

		const int BUFSIZE = 512;
		HANDLE hPipe;
		TCHAR  chBuf[BUFSIZE];
		BOOL   fSuccess = FALSE;
		DWORD  cbRead, cbToWrite, cbWritten, dwMode;
		// Try to open a named pipe; wait for it, if necessary. 

		hPipe = CreateFile(
			TEXT("\\\\.\\pipe\\mynamedpipe"),   // pipe name 
			GENERIC_READ,
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 

		// Break if the pipe handle is valid. 

		if (hPipe == INVALID_HANDLE_VALUE)
			return;

		while(true)
		{
			// Read from the pipe. 

			fSuccess = ReadFile(
				hPipe,    // pipe handle 
				chBuf,    // buffer to receive reply 
				BUFSIZE * sizeof(TCHAR),  // size of buffer 
				&cbRead,  // number of bytes read 
				NULL);    // not overlapped 

			char* data = reinterpret_cast<char*>(chBuf);
			data[cbRead] = 0;

			if (cbRead > 0) {
				std::string setting(data);
				updater->UpdateSettings(setting);
			}
		}

		CloseHandle(hPipe);
	}
}

void rendering::helper::DXUpdater::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

	Value& p_settingsBuffer = GetOrCreateProperty(nativeObject, "settingsBuffer");
	Value& p_hosekSettingsBuffer = GetOrCreateProperty(nativeObject, "hosekSettingsBuffer");
	Value& p_camera = GetOrCreateProperty(nativeObject, "camera");

	Value& setCamera = GetOrCreateProperty(nativeObject, "setCamera");
	setCamera = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXUpdater* self = static_cast<DXUpdater*>(NativeObject::ExtractNativeObject(selfValue));

		Value cameraValue = scope.GetProperty("param0");
		DXCamera* camera = dynamic_cast<DXCamera*>(NativeObject::ExtractNativeObject(cameraValue));
		if (!camera) {
			THROW_EXCEPTION("Please supply Camera!")
		}

		p_camera = cameraValue;
		self->m_camera = camera;

		return Value();
	});

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

	Value& setHosekSettingsBuffer = GetOrCreateProperty(nativeObject, "setHosekSettingsBuffer");
	setHosekSettingsBuffer = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXUpdater* self = static_cast<DXUpdater*>(NativeObject::ExtractNativeObject(selfValue));

		Value bufferValue = scope.GetProperty("param0");
		DXBuffer* buffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(bufferValue));
		if (!buffer) {
			THROW_EXCEPTION("Please supply Hosek Settings buffer!")
		}

		p_hosekSettingsBuffer = bufferValue;
		self->m_hosekSettingsBuffer = buffer->GetBuffer();

		return Value();
	});

	Value& connectToPipe = GetOrCreateProperty(nativeObject, "connectToPipe");
	connectToPipe = CreateNativeMethod(nativeObject, 0, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXUpdater* self = static_cast<DXUpdater*>(NativeObject::ExtractNativeObject(selfValue));

		m_updater = self;
		self->m_pipeThread = new std::thread(run);

		return Value();
	});

	Value& setSettingsFile = GetOrCreateProperty(nativeObject, "setSettingsFile");
	setSettingsFile = CreateNativeMethod(nativeObject, 1, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXUpdater* self = static_cast<DXUpdater*>(NativeObject::ExtractNativeObject(selfValue));

		Value settingsFileValue = scope.GetProperty("param0");
		if (settingsFileValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply setting file name!")
		}
		
		self->m_settingsFile = settingsFileValue.GetString();

		self->UpdateSettings();

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
	std::string dataFromFile = data::GetLibrary().ReadFileByPath(rootDir + m_settingsFile);

	nlohmann::json json = nlohmann::json::parse(dataFromFile);
	const nlohmann::json& settings = json["settings"];
	for (auto it = settings.begin(); it != settings.end(); ++it) {
		const nlohmann::json& cur = *it;
		int dim = static_cast<int>(cur["dim"]);

		Setting setting;
		setting.m_name = it.key();
		setting.m_dim = dim;

		const nlohmann::json& value = cur["value"];
		float val[4];
		if (dim == 1) {
			setting.m_value[0] = static_cast<float>(value);
		}
		else {
			for (int i = 0; i < dim; ++i) {
				setting.m_value[i] = static_cast<float>(value.at(i));
			}
		}
		AddSetting(setting);
	}

	const nlohmann::json& shaderOrdering = json["shaderOrdering"];

	for (auto it = shaderOrdering.begin(); it != shaderOrdering.end(); ++it) {
		m_shaderSettings.push_back(static_cast<std::string>(*it));
	}

	const nlohmann::json& hosekSettings = json["hosekSettings"];
	for (auto it = hosekSettings.begin(); it != hosekSettings.end(); ++it) {
		m_hosekSettings.push_back(static_cast<std::string>(*it));
	}
}

void rendering::helper::DXUpdater::AddSetting(const Setting& setting)
{
	m_settings.insert(std::pair<std::string, Setting>(setting.m_name, setting));
}

void rendering::helper::DXUpdater::UpdateSettings(const std::string& setting)
{
	std::stringstream ss(setting);
	std::string settingName;

	while (ss >> settingName) {
		Setting& setting = m_settings[settingName];
		for (int i = 0; i < setting.m_dim; ++i) {
			ss >> setting.m_value[i];
		}
	}
}

void rendering::helper::DXUpdater::Update(double dt)
{
	using namespace interpreter;
	using namespace DirectX;
	m_totalTime += dt;

	const Setting& sunAzimuth = m_settings["m_sunAzimuth"];
	const Setting& sunAltitude = m_settings["m_sunAltitude"];

	DXCamera* cam = static_cast<DXCamera*>(m_camera);
	cam->m_sunAzimuth = sunAzimuth.m_value[0];
	cam->m_sunAltitude = sunAltitude.m_value[0];

	CD3DX12_RANGE readRange(0, 0);
	void* dst = nullptr;

	if (m_hosekSettingsBuffer) {
		HRESULT hr = m_hosekSettingsBuffer->Map(0, &readRange, &dst);
		if (FAILED(hr)) {
			return;
		}
		float* floatData = reinterpret_cast<float*>(dst);

		for (std::list<std::string>::iterator it = m_hosekSettings.begin(); it != m_hosekSettings.end(); ++it) {
			const Setting& cur = m_settings[*it];

			for (int i = 0; i < cur.m_dim; ++i) {
				*floatData = cur.m_value[i];
				++floatData;
			}
		}

		m_hosekSettingsBuffer->Unmap(0, nullptr);
	}

	HRESULT hr = m_settingsBuffer->Map(0, &readRange, &dst);
	if (FAILED(hr)) {
		return;
	}
	float* floatData = reinterpret_cast<float*>(dst);

	for (std::list<std::string>::iterator it = m_shaderSettings.begin(); it != m_shaderSettings.end(); ++it) {
		const Setting& cur = m_settings[*it];

		for (int i = 0; i < cur.m_dim; ++i) {
			*floatData = cur.m_value[i];
			++floatData;
		}
	}

	m_settingsBuffer->Unmap(0, nullptr);
}

rendering::helper::DXUpdater::~DXUpdater()
{
	if (m_pipeThread) {
		delete m_pipeThread;
	}
	m_pipeThread = nullptr;
}

#undef THROW_ERROR
