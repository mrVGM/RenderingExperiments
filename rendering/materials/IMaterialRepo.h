#pragma once

#include <string>
#include <map>

namespace rendering::material
{
	class IMaterial;

	class IMaterialRepo
	{
	public:
		std::map<std::string, IMaterial*> m_materials;
	};
}