#pragma once

#include <nex/gui/Drawable.hpp>
#include <nex/scene/Scene.hpp>


namespace nex
{
	class Scene;
	class Vob;
}

namespace nex::gui
{

	class Picker;

	class VobView {
	public:
		virtual ~VobView() = default;

		/**
		 * @return : true if the vob is still selected
		 */
		virtual void draw(Vob* vob, Scene* scene, Picker* picker, bool doOneTimeChanges);
	};
}