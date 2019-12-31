#pragma once

#include <nex/gui/Drawable.hpp>
#include <nex/scene/Scene.hpp>


namespace nex
{
	class Scene;
	class Window;
	class Vob;
	class ProbeVob;
	class Camera;
}

namespace nex::gui
{
	class Picker;
	class VobView;

	class VobEditor : public nex::gui::Drawable
	{
	public:
		VobEditor(nex::Window* window, Picker* picker, Camera* camera, float splitterPosition = 300.0f);
		virtual ~VobEditor();
		void setScene(nex::Scene* scene);
		void setVobView(VobView* view);

	protected:

		void drawSelf() override;

		Vob* drawVobHierarchy(Vob* vob);

		nex::Vob* mLastPickedVob;
		
		VobView* mVobView;
		Picker* mPicker;

		nex::Window* mWindow;
		nex::Scene* mScene;
		Camera* mCamera;
		float mSplitterPosition;
		bool mInit;
		float mInitialHeight;
		float mInitialSplitPosition;
		//TextureView mTransparentView;
	};
}