#pragma once

#include "nativeObject.h"

#include <map>
#include <string>

namespace rendering
{
	class CloudSettingsReader : public interpreter::INativeObject
	{
		std::map<std::string, float> m_settingsMap;
		void InitProperties(interpreter::NativeObject& nativeObject) override;
	public:
		CloudSettingsReader();
	};
}
