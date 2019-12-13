#pragma once

#include <nex/resource/Resource.hpp>
#include <nex/common/Future.hpp>

namespace nex {
	class Window;
}

namespace nex::gui
{
	class TextureLoader
	{
	public:
		TextureLoader(nex::Window* window);

		nex::Future<Resource*> selectTexture();

	protected:
		nex::Window* mWindow;
	};
}