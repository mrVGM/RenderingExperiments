#pragma once

#include "nativeObject.h"

#include <map>
#include <string>

namespace rendering
{
	class CloudSettingsReader : public interpreter::INativeObject
	{
		void InitProperties(interpreter::NativeObject& nativeObject) override;
	public:
	};
}
