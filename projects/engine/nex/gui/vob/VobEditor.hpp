#pragma once

#include <nex/gui/Drawable.hpp>
#include <nex/scene/Scene.hpp>
#include <nex/gui/ImGUI.hpp>
#include <nex/gui/vob/VobView.hpp>
#include <nex/gui/SceneView.hpp>

namespace nex
{
	class Scene;
	class Window;
	class Vob;
	class Camera;
}

namespace nex::gui
{
	class Picker;
	class VobView;

	class VobEditor : public nex::gui::Drawable
	{
	public:
		VobEditor(nex::Window* window, Picker* picker, Scene* scene, Camera* camera, float splitterPosition = 300.0f);
		virtual ~VobEditor();

		void updateVobView(Vob* pickedVob);

		void setVisible(bool visible) override;

	protected:

		void drawSelf() override;

		void drawRightSideContent();

		nex::Vob* mLastPickedVob;
		
		VobView* mVobView;
		Picker* mPicker;

		nex::Window* mWindow;
		nex::Scene* mScene;
		Camera* mCamera;
		ImVec2 mLeftMinSize;
		bool mInit;

		ImVec2 mLeftContentSize;
		ImVec2 mRightContentSize;

		SceneView mSceneView;
	};
}