#pragma once

#include "nativeObject.h"

#include <DirectXMath.h>
#include <map>
#include <string>

namespace rendering::scene
{
	struct Transform
	{
		DirectX::XMVECTOR m_position = DirectX::XMVectorSet(0,0,0,1);
		DirectX::XMVECTOR m_rotation = DirectX::XMVectorSet(1, 0, 0, 0);
		DirectX::XMVECTOR m_scale = DirectX::XMVectorSet(1, 1, 1, 1);;
	};

	struct Object3D
	{
		std::string m_mesh;
		Transform m_transform;
		std::list<std::string> m_materials;
	};

	class DXScene : public interpreter::INativeObject
	{
		std::map<std::string, Object3D> m_objects;
		void InitProperties(interpreter::NativeObject& nativeObject) override;
	public:
	};
}