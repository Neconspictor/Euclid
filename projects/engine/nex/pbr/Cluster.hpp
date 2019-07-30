#pragma once

#include <nex/gui/MenuWindow.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/Scene.hpp>

namespace nex
{

	class ProbeCluster
	{
	public:

		ProbeCluster(Scene* scene);

		nex::PerspectiveCamera& getCamera();

		void generate();

	private:

		nex::PerspectiveCamera mCamera;
		Scene* mScene;
	};

	namespace gui {

		class ProbeClusterView : public nex::gui::MenuWindow {
		public:
			ProbeClusterView(std::string title,
				MainMenuBar* menuBar,
				Menu* menu, ProbeCluster* cluster,
				PerspectiveCamera* activeCamera);

			void drawSelf() override;
		
		private:
			ProbeCluster* mCluster;
			PerspectiveCamera* mActiveCamera;
		};
	}
}