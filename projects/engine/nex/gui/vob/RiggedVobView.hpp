#pragma once

#include <nex/gui/vob/VobView.hpp>
#include <nex/common/Future.hpp>

namespace nex {
	class Resource;
	class Window;
}

namespace nex::gui
{

	class RiggedVobView : public VobView {
	public:

		RiggedVobView(nex::Window* window);
		virtual ~RiggedVobView() = default;

		bool draw(Vob* vob, Scene* scene, Picker* picker, Camera* camera, bool doOneTimeChanges) override;

	protected:

		void drawLoadAni();

		Future<Resource*> mResourceFuture;
		Future<Resource*>  loadAnimation();
		nex::Window* mWindow;
	};
}