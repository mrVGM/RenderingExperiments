#pragma once

#include "nativeObject.h"
#include "collada.h"

#include <d3d12.h>
#include <DirectXMath.h>
#include <map>
#include <string>

namespace rendering::scene
{
	struct Transform
	{
		float m_position[3] = { 0, 0, 0 };
		float m_rotation[4] = { 1, 0, 0, 0 };
		float m_scale[3] = { 1, 1, 1 };
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

	struct ColladaGeometryBuffers
	{
		ID3D12Resource* m_vertexBuffer = nullptr;
		ID3D12Resource* m_indexBuffer = nullptr;
	};

	class IScene
	{
	public:
		std::map<std::string, Object3D> m_objects;
		std::map<int, InstanceBuffer> m_instanceBuffers;

		std::map<std::string, ColladaGeometryBuffers> m_colladaGeometryBuffers;
		std::map<std::string, ID3D12Resource*> m_colladaInstanceBuffers;
		collada::Scene m_colladaScene;
	};
}