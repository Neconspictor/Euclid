#pragma once

#include <nex/gui/MenuWindow.hpp>
#include <nex/gui/TextureView.hpp>


namespace nex
{
	class Scene;
	class Window;
	class Vob;
}

namespace nex::gui
{
	class TextureViewer : public nex::gui::MenuWindow
	{
	public:
		TextureViewer(std::string title, nex::gui::MainMenuBar* menuBar, nex::gui::Menu* menu, nex::Window* widow);
		virtual ~TextureViewer();

	protected:

		void drawSelf() override;
        
		TextureView mDynamicLoad;
		nex::Window* mWindow;
	};
}