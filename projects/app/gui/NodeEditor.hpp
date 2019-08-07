#pragma once

#include <nex/gui/Drawable.hpp>
#include "nex/gui/TextureView.hpp"


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

	class NodeEditor : public nex::gui::Drawable
	{
	public:
		NodeEditor(nex::Window* window);
		virtual ~NodeEditor();
		void setPicker(Picker* picker);
		void setScene(nex::Scene* scene);

	protected:

		void drawSelf() override;

		void drawProbeVob(nex::ProbeVob* vob, bool doOneTimeChanges);

		nex::Vob* mLastPicked;
		Picker* mPicker;
		TextureView mBrdfView;
		TextureView mConvolutedView;
		TextureView mPrefilteredView;
		nex::Window* mWindow;
		nex::Scene* mScene;
		//TextureView mTransparentView;
	};
}