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

		m_floats.push_back(Value(self->m_settingsMap["cs_SampleStep"]));
		m_floats.push_back(Value(self->m_settingsMap["cs_MaxSampleSteps"]));

		m_floats.push_back(Value(self->m_settingsMap["cs_WeightR"]));
		m_floats.push_back(Value(self->m_settingsMap["cs_WeightG"]));
		m_floats.push_back(Value(self->m_settingsMap["cs_WeightB"]));

		m_floats.push_back(Value(self->m_settingsMap["cs_DensityOffset"]));

		m_floats.push_back(Value(self->m_settingsMap["cs_DensityThresholdR"]));
		m_floats.push_back(Value(self->m_settingsMap["cs_DensityMultiplierR"]));
		m_floats.push_back(Value(self->m_settingsMap["cs_UVWFactorR"]));

		m_floats.push_back(Value(self->m_settingsMap["cs_DensityThresholdG"]));
		m_floats.push_back(Value(self->m_settingsMap["cs_DensityMultiplierG"]));
		m_floats.push_back(Value(self->m_settingsMap["cs_UVWFactorG"]));

		m_floats.push_back(Value(self->m_settingsMap["cs_DensityThresholdB"]));
		m_floats.push_back(Value(self->m_settingsMap["cs_DensityMultiplierB"]));
		m_floats.push_back(Value(self->m_settingsMap["cs_UVWFactorB"]));

		return Value::FromList(m_floats);
	});

#undef THROW_EXCEPTION
}

rendering::CloudSettingsReader::CloudSettingsReader()
{
	m_settingsMap.insert(std::pair<std::string, float>("cs_SampleStep", 0.01));
	m_settingsMap.insert(std::pair<std::string, float>("cs_MaxSampleSteps", 300));

	m_settingsMap.insert(std::pair<std::string, float>("cs_WeightR", 1));
	m_settingsMap.insert(std::pair<std::string, float>("cs_WeightG", 0));
	m_settingsMap.insert(std::pair<std::string, float>("cs_WeightB", 0));

	m_settingsMap.insert(std::pair<std::string, float>("cs_DensityOffset", -1));

	m_settingsMap.insert(std::pair<std::string, float>("cs_DensityThresholdR", 0.4));
	m_settingsMap.insert(std::pair<std::string, float>("cs_DensityMultiplierR", 2));
	m_settingsMap.insert(std::pair<std::string, float>("cs_UVWFactorR", 0.1));

	m_settingsMap.insert(std::pair<std::string, float>("cs_DensityThresholdG", 0.4));
	m_settingsMap.insert(std::pair<std::string, float>("cs_DensityMultiplierG", 2));
	m_settingsMap.insert(std::pair<std::string, float>("cs_UVWFactorG", 0.1));

	m_settingsMap.insert(std::pair<std::string, float>("cs_DensityThresholdB", 0.4));
	m_settingsMap.insert(std::pair<std::string, float>("cs_DensityMultiplierB", 2));
	m_settingsMap.insert(std::pair<std::string, float>("cs_UVWFactorB", 0.1));
}

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}


#undef THROW_ERROR
