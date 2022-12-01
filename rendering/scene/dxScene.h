#pragma once

#include "nativeObject.h"

#include <d3d12.h>
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
		int m_instanceBufferID = -1;
		int m_instanceBufferOffset = -1;

		std::string m_mesh;
		Transform m_transform;
		std::list<std::string> m_materials;

		bool Similar(const Object3D& other) const;
	};

	struct InstanceBuffer
	{
		int m_size = 0;
		int m_stride = 0;
		ID3D12Resource* m_buffer = nullptr;
		int m_objectsCount = 0;
	};

	class DXScene : public interpreter::INativeObject
	{
		int m_instanceBufferCount = 0;
		std::map<std::string, Object3D> m_objects;
		std::map<int, InstanceBuffer> m_instanceBuffers;

		void InitProperties(interpreter::NativeObject& nativeObject) override;

		void ConstructInstanceBuffersData();
	public:
	};
}