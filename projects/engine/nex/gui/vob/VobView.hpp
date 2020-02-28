#pragma once

#include <nex/gui/Drawable.hpp>
#include <nex/scene/Scene.hpp>
#include <nex/gui/ImGUI.hpp>
#include<nex/texture/Texture.hpp>
#include <nex/texture/TextureManager.hpp>


namespace nex
{
	class Scene;
	class Vob;
	class Camera;
	class Window;
}

namespace nex::gui
{

	class Picker;

	class VobView {
	public:

		VobView(nex::Window* window);
		virtual ~VobView() = default;

		/**
		 * @return : true if the vob is still valid
		 */
		virtual bool draw(Vob* vob, 
			Scene* scene, 
			Picker* picker, 
			Camera* camera,
			bool doOneTimeChanges);

		void drawIcon();


		bool hasIcon() const;
		const ImGUI_TextureDesc& getIconDesc() const;

		bool centerIconHeight() const;
		const ImVec4& getIconTintColor() const;

	protected:
		ImGUI_TextureDesc mIconDesc;
		ImVec4 mIconTintColor = ImVec4(0.29f, 1.0f, 0.59f, 1.0f);
		bool mCenterIconHeight = false;

		void drawKeyFrameAni(nex::Vob* vob);
		Future<Resource*> mKeyFrameAniFuture;
		Future<Resource*>  loadKeyFrameAnimation(nex::Vob* vob);
		nex::Window* mWindow;
	};
}