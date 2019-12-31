#pragma once
#include <nex/gui/Drawable.hpp>
#include <nex/camera/Camera.hpp>

namespace nex
{
	class FPCamera_ConfigurationView;

	class FPCamera : public PerspectiveCamera
	{
	public:
		FPCamera(float width, float height);

		void setLook(glm::vec3 look) override;
		void frameUpdate(Input* input, float frameTime) override;

		float getYaw() const;
		float getPitch() const;

		void recalculateLookVector();

		/**
		 * Sets yaw angle in degree
		 */
		void setYaw(float yaw);
		
		/**
		 * Sets pitch angle in degree
		 */
		void setPitch(float pitch);

	protected:

		friend FPCamera_ConfigurationView;

		static float limit(float source, float min, float max);

		float mYaw, mPitch;
	};

	class FPCamera_ConfigurationView : public nex::gui::Drawable {
	public:
		FPCamera_ConfigurationView(FPCamera* camera);

	protected:
		void drawSelf() override;

	private:
		FPCamera * mCamera;
	};
}
