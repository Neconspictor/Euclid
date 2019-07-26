#pragma once

#include <nex/gui/Drawable.hpp>
#include "nex/gui/TextureView.hpp"


namespace nex
{
	class Scene;
	class Window;
	class Vob;
}

namespace nex::gui
{
	class Picker;

	class NodeEditor : public nex::gui::Drawable
	{
	public:
		NodeEditor(nex::Window* window);
		virtual ~NodeEditor();
		void setPicker(Picker* picker);
		void setScene(nex::Scene* scene);

	protected:

		void drawSelf() override;

		nex::Vob* mLastPicked;
		Picker* mPicker;
		TextureView mBrdfView;
		TextureView mConvolutedView;
		TextureView mPrefilteredView;
		TextureView mDynamicLoad;
		nex::Window* mWindow;
		nex::Scene* mScene;
		//TextureView mTransparentView;
	};
}