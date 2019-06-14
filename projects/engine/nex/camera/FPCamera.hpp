#pragma once
#include <nex/gui/Drawable.hpp>
#include <nex/camera/Camera.hpp>

namespace nex
{
	class FPCamera_ConfigurationView;

	class FPCamera : public PerspectiveCamera
	{
	public:
		FPCamera(unsigned width, unsigned height);

		void setLook(glm::vec3 look) override;
		void frameUpdate(Input* input, Real frameTime) override;

		Real getYaw() const;
		Real getPitch() const;

		void recalculateLookVector();

		/**
		 * Sets yaw angle in degree
		 */
		void setYaw(Real yaw);
		
		/**
		 * Sets pitch angle in degree
		 */
		void setPitch(Real pitch);

	protected:

		friend FPCamera_ConfigurationView;

		static Real limit(Real source, Real min, Real max);

		Real yaw, pitch;
	};

	class FPCamera_ConfigurationView : public nex::gui::Drawable {
	public:
		FPCamera_ConfigurationView(FPCamera* camera);

	protected:
		void drawSelf() override;

	private:
		FPCamera * m_camera;
	};
}
