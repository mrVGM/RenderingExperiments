#include "helper/cloudSettingsReader.h"

#include "nativeFunc.h"

#include "dxBuffer.h"

#include <sstream>
#include <map>
#include <list>

void rendering::CloudSettingsReader::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

	Value& readCloudSettings = GetOrCreateProperty(nativeObject, "readCloudSettings");
	readCloudSettings = CreateNativeMethod(nativeObject, 1, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		CloudSettingsReader* self = static_cast<CloudSettingsReader*>(NativeObject::ExtractNativeObject(selfValue));

		Value settingsDataValue = scope.GetProperty("param0");
		if (settingsDataValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply a Settings Data!")
		}

		std::stringstream ss(settingsDataValue.GetString());

		std::string key;
		float value;
		while (ss >> key) {
			ss >> value;
			self->m_settingsMap[key] = value;
		}

		std::list<Value> m_floats;
		m_floats.push_back(Value(self->m_settingsMap["cs_DensityOffset"]));
		m_floats.push_back(Value(self->m_settingsMap["cs_DensityThreshold"]));
		m_floats.push_back(Value(self->m_settingsMap["cs_DensityMultiplier"]));
		m_floats.push_back(Value(self->m_settingsMap["cs_UVWFactor"]));

		return Value::FromList(m_floats);
	});

#undef THROW_EXCEPTION
}

rendering::CloudSettingsReader::CloudSettingsReader()
{
	m_settingsMap.insert(std::pair<std::string, float>("cs_DensityOffset", -1));
	m_settingsMap.insert(std::pair<std::string, float>("cs_DensityThreshold", 0.4));
	m_settingsMap.insert(std::pair<std::string, float>("cs_DensityMultiplier", 2));
	m_settingsMap.insert(std::pair<std::string, float>("cs_UVWFactor", 0.1));
}

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}


#undef THROW_ERROR
