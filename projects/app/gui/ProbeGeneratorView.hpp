#pragma once

#include <nex/gui/MenuWindow.hpp>
#include <glm/glm.hpp>


namespace nex
{
	class Scene;
	class Window;
}

namespace nex::gui
{
	class ProbeGeneratorView : public nex::gui::MenuWindow
	{
	public:
		ProbeGeneratorView(std::string title, nex::gui::MainMenuBar* menuBar, nex::gui::Menu* menu, nex::Scene* scene);
		virtual ~ProbeGeneratorView();
		void setScene(nex::Scene* scene);

	protected:

		void drawSelf() override;

		nex::Scene* mScene;
		glm::vec3 mPosition;
	};
}