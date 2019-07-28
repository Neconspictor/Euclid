#pragma once

#include <nex/gui/MenuWindow.hpp>
#include <glm/glm.hpp>


namespace nex
{
	class Scene;
	class Window;
	class ProbeGenerator;
	class Camera;
}

namespace nex::gui
{
	class ProbeGeneratorView : public nex::gui::MenuWindow
	{
	public:
		ProbeGeneratorView(std::string title, nex::gui::MainMenuBar* menuBar, nex::gui::Menu* menu, nex::ProbeGenerator* generator,
			nex::Camera* camera);
		virtual ~ProbeGeneratorView();
		void setVisible(bool visible) override;

	protected:

		void drawSelf() override;

		nex::ProbeGenerator* mGenerator;
		nex::Camera* mCamera;
		float mPlacementOffset;
	};
}