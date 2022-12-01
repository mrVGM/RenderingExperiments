#pragma once

#include "nativeObject.h"
#include "materials/IMaterial.h"
#include "materials/IMaterialRepo.h"

#include <map>
#include <string>

namespace rendering::material
{
	class DXMaterialRepo : public interpreter::INativeObject, public IMaterialRepo
	{
		void InitProperties(interpreter::NativeObject& nativeObject) override;
	public:
	};
}