#pragma once
#include <nex/camera/FPCameraBase.hpp>
#include <nex/gui/Drawable.hpp>

namespace nex
{
	class FPCamera_ConfigurationView;

	class FPCamera : public FPCameraBase
	{
	public:
		FPCamera();
		FPCamera(glm::vec3 position, glm::vec3 look, glm::vec3 up);
		FPCamera(const FPCamera& other);


		float limit(float source, float min, float max);
		virtual void setLook(glm::vec3 direction) override;
		virtual void update(Input* input, float frameTime) override;

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


		float yaw, pitch;
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