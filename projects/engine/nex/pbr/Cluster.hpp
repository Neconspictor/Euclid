#pragma once

#include <nex/gui/MenuWindow.hpp>
#include <nex/camera/Camera.hpp>

namespace nex
{

	class ProbeCluster
	{
	public:

		ProbeCluster();

		nex::PerspectiveCamera& getCamera();

	private:

		nex::PerspectiveCamera mCamera;
	};

	namespace gui {

		class ProbeClusterView : public nex::gui::MenuWindow {
		public:
			ProbeClusterView(std::string title,
				MainMenuBar* menuBar,
				Menu* menu, ProbeCluster* cluster);

			void drawSelf() override;
		
		private:
			ProbeCluster* mCluster;
		};
	}
}