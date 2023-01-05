#pragma once

#include "nativeObject.h"
#include "scene/IScene.h"

#include <d3d12.h>
#include <DirectXMath.h>
#include <map>
#include <string>

namespace rendering::scene
{
	class DXScene : public interpreter::INativeObject, public IScene
	{
		int m_instanceBufferCount = 0;
		void InitProperties(interpreter::NativeObject& nativeObject) override;

		void ConstructInstanceBuffersData();

		bool ReadColladaScene(const std::string& colladaFile, std::string& errorMessage);
	public:
		~DXScene();
	};
}