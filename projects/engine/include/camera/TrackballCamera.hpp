#ifndef CAMERA_TRACKBALL_CAMERA_HPP
#define CAMERA_TRACKBALL_CAMERA_HPP
#include <camera/Camera.hpp>

class TrackballCamera : public Camera
{
public:
	TrackballCamera();
	TrackballCamera(glm::vec3 trackPosition, float polarAngle, float azimuthAngle, float radius);
	TrackballCamera(const TrackballCamera& other);
	virtual ~TrackballCamera();

	float getAzimuthAngle() const;
	float getPolarAngle() const;
	float getRadius() const;
	const glm::vec3& getTrackPosition() const;

	virtual void update(int mouseXFrameOffset, int mouseYFrameOffset) override;

protected:
	float polarAngle, azimuthAngle, radius;
	glm::vec3 trackPosition;

	glm::vec3 sphericalCoordinatesToCartesian() const;
	void updateCartesianCoordinates();

	/*
	some stuff for converting cartesian coordinates to spherical coordinates:
	y/ radius = cos(polarAngle)
	cos^-1(y/radius) = polarAngle

	z/(radius* (1 - (y/radius)) = cos(azimuthAngle)
	cos^-1(z/(radius* (1 - (y/radius))) = azimuthAngle;
	*/
};

#endif