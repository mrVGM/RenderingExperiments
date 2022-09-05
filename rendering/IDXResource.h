#pragma once

#include <d3d12.h>

namespace rendering
{
	class IDXResource
	{
	public:
		virtual ID3D12Resource* GetResource() const = 0;
	};
}