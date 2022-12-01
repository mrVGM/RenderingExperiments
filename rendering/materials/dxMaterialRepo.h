#pragma once

#include "nativeObject.h"
#include "materials/IMaterial.h"

#include <map>
#include <string>

namespace rendering::material
{
	class DXMaterialRepo : public interpreter::INativeObject
	{
		std::map<std::string, IMaterial*> m_materials;
		void InitProperties(interpreter::NativeObject& nativeObject) override;
	public:
	};
}