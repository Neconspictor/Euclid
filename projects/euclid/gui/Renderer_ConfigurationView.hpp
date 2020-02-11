#pragma once

#include <nex/gui/Drawable.hpp>
#include <EuclidRenderer.hpp>


namespace nex
{
	class Camera;
	class PerspectiveCamera;
	class Input;
	class Scene;
	class SceneNode;
	class Window;
	class Vob;
}

namespace nex::gui
{
	class Renderer_ConfigurationView : public nex::gui::Drawable
	{
	public:
		Renderer_ConfigurationView(EuclidRenderer* renderer);

	protected:
		void drawSelf() override;

		EuclidRenderer* mRenderer;
	};
}