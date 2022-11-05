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
		std::list<Value> m_floats;

		while (ss >> key) {
			ss >> value;
			m_floats.push_back(Value(value));
		}

		return Value::FromList(m_floats);
	});

#undef THROW_EXCEPTION
}

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}


#undef THROW_ERROR
