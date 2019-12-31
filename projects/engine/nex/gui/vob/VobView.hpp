#pragma once

#include <nex/gui/Drawable.hpp>
#include <nex/scene/Scene.hpp>


namespace nex
{
	class Scene;
	class Vob;
	class Camera;
}

namespace nex::gui
{

	class Picker;

	class VobView {
	public:
		virtual ~VobView() = default;

		/**
		 * @return : true if the vob is still valid
		 */
		virtual bool draw(Vob* vob, 
			Scene* scene, 
			Picker* picker, 
			Camera* camera,
			bool doOneTimeChanges);
	};
}