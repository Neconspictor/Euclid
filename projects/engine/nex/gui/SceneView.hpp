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
	class Vob;
}

namespace nex::gui
{
	class Picker;
	class VobView;

	class SceneView : public nex::gui::Drawable
	{
	public:
		SceneView(Picker* picker, Scene* scene);
		virtual ~SceneView();
		void setScene(nex::Scene* scene);

	protected:

		class VobDrawer {
		public:
			virtual ~VobDrawer() = default;
			virtual Vob* draw(SceneView* sceneView, Vob* vob) = 0;

		protected:
			bool mCurrentSelctedIsEditing = false;
			Vob* mEditedVob = nullptr;
			std::string mNameBackup = "";
		};

		class VobWithChildrenDrawer : public VobDrawer {
		public:
			Vob* draw(SceneView* sceneView, Vob* vob) override;

		protected:
			bool drawEditing(const char* label, SceneView* sceneView, Vob* vob, bool& popTree, bool& returnVob);
			bool drawNormal(const char* label, SceneView* sceneView, Vob* vob, bool& popTree, bool& returnVob);
		};

		class VobWithoutChildrenDrawer : public VobDrawer {
		public:
			Vob* draw(SceneView* sceneView, Vob* vob) override;


		private:
			bool mCurrentSelctedIsEditing;
		};

		void drawSelf() override;

		Vob* drawVobHierarchy(Vob* vob);

		Vob* drawVobWithChildren(Vob* vob);

		bool drawSceneRoot();
		void endSceneRoot();
		void drawDragDrop(Vob* vob);
		void drawDragDropRoot();
		void drawDragDropRearrange(int placerIndex);
		static void drawCustomRootHeader(ImGuiID id, ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool border, float rounding);
		static void drawCustomVobHeader(ImGuiID id, ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool border, float rounding);

		void drawIcon(const ImGUI_TextureDesc* desc, bool centerHeight, const ImVec4& tintColor);
		
		Picker* mPicker;
		nex::Scene* mScene;
		static constexpr const char* VOB_PAYLOAD = "vob";

		ImGUI_TextureDesc mSceneRootIcon;
		VobWithoutChildrenDrawer mVobWithoutChildrenDrawer;
		VobWithChildrenDrawer mVobWithChildrenDrawer;
	};
}