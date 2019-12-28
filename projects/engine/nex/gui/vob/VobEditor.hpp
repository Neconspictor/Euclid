#pragma once

#include <nex/gui/Drawable.hpp>
#include <nex/scene/Scene.hpp>


namespace nex
{
	class Scene;
	class Window;
	class Vob;
	class ProbeVob;
}

namespace nex::gui
{
	class Picker;
	class VobView;

	class VobEditor : public nex::gui::Drawable
	{
	public:
		VobEditor(nex::Window* window, Picker* picker);
		virtual ~VobEditor();
		void setScene(nex::Scene* scene);
		void setVobView(VobView* view);

	protected:

		void drawSelf() override;

		void drawProbeVob(nex::ProbeVob* vob, bool doOneTimeChanges);

		nex::Vob* mLastPickedVob;
		
		VobView* mVobView;
		Picker* mPicker;

		nex::Window* mWindow;
		nex::Scene* mScene;
		//TextureView mTransparentView;
	};
}