#pragma once

#include <nex/gui/Drawable.hpp>
#include <nex/scene/Scene.hpp>
#include <nex/gui/ImGUI.hpp>

#include <nex/gui/vob/VobView.hpp>
#include <nex/gui/vob/PbrProbeVobView.hpp>
#include <nex/gui/vob/OceanVobView.hpp>
#include <nex/gui/vob/RiggedVobView.hpp>



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

		void updateVobView(Vob* pickedVob);
		nex::gui::VobView* getViewByVob(Vob* vob);

		void setVisible(bool visible) override;

	protected:

		void drawSelf() override;

		Vob* drawVobHierarchy(Vob* vob);

		void drawScene();
		bool drawSceneRoot();
		void endSceneRoot();
		void drawDragDrop(Vob* vob);
		void drawDragDropRoot();
		void drawDragDropRearrange(int placerIndex);
		static void drawCustomRootHeader(ImGuiID id, ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool border, float rounding);
		static void drawCustomVobHeader(ImGuiID id, ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool border, float rounding);

		void drawIcon(const ImGUI_TextureDesc* desc, bool centerHeight, const ImVec4& tintColor);

		nex::Vob* mLastPickedVob;
		
		VobView* mVobView;
		Picker* mPicker;

		nex::Window* mWindow;
		nex::Scene* mScene;
		Camera* mCamera;
		float mSplitterPosition;
		ImVec2 mLeftMinSize;
		bool mInit;
		float mInitialHeight;
		float mInitialSplitPosition;

		ImVec2 mLeftContentSize;
		ImVec2 mRightContentSize;
		ImVec2 mLeftContentPadding;


		nex::gui::VobView mDefaultVobView;
		nex::gui::PbrProbeVobView mProbeVobView;
		nex::gui::OceanVobView mOceanVobView;
		nex::gui::RiggedVobView mRiggedVobView;

		ImGUI_TextureDesc mSceneRootIcon;

		//TextureView mTransparentView;
	};
}