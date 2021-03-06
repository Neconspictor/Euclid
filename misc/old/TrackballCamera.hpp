#pragma once
#include <nex/camera/Camera.hpp>

namespace nex
{

	class TrackballCamera : public Camera
	{
	public:

		struct SphericalCoord
		{
			float polar;
			float azimuth;
			float radius;
		};

		TrackballCamera();
		TrackballCamera(glm::vec3 trackPosition, float polarAngle, float azimuthAngle, float radius);
		TrackballCamera(const TrackballCamera& other);

		float getAzimuthAngle() const;
		float getPolarAngle() const;
		float getRadius() const;
		const glm::vec3& getTrackPosition() const;

		/**
		 * Pans camera on the x-z plane.
		 */
		virtual void pan(float xPos, float zPos);

		/**
		 * Rotates this camera on the specified polar and azimuth angle.
		 * NOTE: polar and azimuth are both assumed to be radian angles.
		 */
		virtual void rotate(float polar, float azimuth);

		void setAzimuthAngle(float azimuth);


		/**
		 * Setting the look direction manually makes no sense for a Trackball camera,
		 * So this function does nothing!
		 */
		void setLook(glm::vec3 direction) override;

		void setPolarAngle(float polar);

		/**
		 * Updates the position of the camera. The camera will still look on the track
		 * position after this function call.
		 */
		void setPosition(glm::vec3 position) override;

		void setRadius(float radius);

		void setTrackPosition(glm::vec3 trackPosition);

		/**
		 * A trackball camera assumes the positive y axis as the up direction.
		 * Therefore this function override does nothing change on this camera.
		 */
		void setUp(glm::vec3 up) override;

		void update(Input* input, float frameTime) override;

	protected:
		SphericalCoord coords;
		glm::vec3 trackPosition;

		/**
		 * Converts cartesian position coordinates (x,y,z) to spherical coordindates (polar, azimuth, radius)
		 * NOTE: This function assumes a right handed coordination system with the positive y-axis as
		 * the up direction.
		 */
		SphericalCoord cartesianToSpherical(const glm::vec3& position, const glm::vec3& lookTarget) const;

		/**
		 * Converts spherical coordindates (polar, azimuth, radius) to cartesian coordinates (x,y,z)
		 * NOTE: This function assumes a right handed coordination system with the positive y-axis as
		 * the up direction.
		 */
		glm::vec3 sphericalToCartesian(SphericalCoord sphericalCoord, const glm::vec3& lookTarget = { 0,0,0 }) const;

		/**
		 * Sets the value of a given pointer to a radian angle.
		 */
		void updateAzimuth(float* sourceAngle, float newValue) const;
		void updatePolar(float* sourceAngle, float newValue) const;
		void updateCartesianCoordinates();
		void updateSphericalCoords();
	};
}