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

		void drawSelf() override;

		Vob* drawVobHierarchy(Vob* vob);

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
	};
}