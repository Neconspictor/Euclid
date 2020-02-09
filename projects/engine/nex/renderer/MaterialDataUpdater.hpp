#pragma once

#include <glm/glm.hpp>
#include <interface/buffers.h>
#include <nex/renderer/RenderCommandQueue.hpp>

namespace nex
{
	class ShaderBuffer;
	class Scene;


	class MaterialDataUpdater
	{
	public:
		static void updateMaterialData(Scene* scene, ShaderBuffer* materialBuffer);
	};
}