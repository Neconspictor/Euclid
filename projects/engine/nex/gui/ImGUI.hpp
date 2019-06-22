#pragma once

#include <imgui/imgui.h>
#include <nex/math/Constant.hpp>

namespace nex
{
	class Texture;
	class Sampler;
}

namespace nex::gui
{


	/**
	 * An interface for concrete render implementations of ImGUI
	 * https://github.com/ocornut/imgui
	 */
	class ImGUI_Impl
	{
	public:
		virtual ~ImGUI_Impl() = default;

		ImGUI_Impl() = default;
		ImGUI_Impl(const ImGUI_Impl&) = delete;
		ImGUI_Impl& operator=(const ImGUI_Impl&) = delete;

		ImGUI_Impl(ImGUI_Impl&&) = default;
		ImGUI_Impl& operator=(ImGUI_Impl&&) = default;

		virtual void newFrame(float frameTime) = 0;

		static bool isActive();

		virtual void renderDrawData(ImDrawData* draw_data) = 0;
	};

	struct ImGUI_ImageDesc
	{
		// the texture to dispaly. Mustn't be nullptr
		Texture* texture;

		// can be nullptr - then a default sampler object is used
		Sampler* sampler;

		// level of Detail (mipmap); only used if the sampler's minfilter supports mipmapping.
		unsigned lod;
		// Only for texture arrays, 3d textures or cubemaps. 
		// Specify the index for a texture array, the depth for a 3d texture or the side of a cubemap.
		unsigned level; 
	};
}