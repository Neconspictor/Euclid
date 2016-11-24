#include <camera/TrackballCamera.hpp>

using namespace std;
using namespace glm;

TrackballCamera::TrackballCamera() : Camera(), polarAngle(0), azimuthAngle(0), radius(100), trackPosition(0,0,0)
{
	updateCartesianCoordinates();
}

TrackballCamera::TrackballCamera(vec3 trackPosition, float polarAngle, float azimuthAngle, float radius) : Camera(), 
polarAngle(polarAngle), azimuthAngle(azimuthAngle), radius(radius), trackPosition(trackPosition)
{
	updateCartesianCoordinates();
}

TrackballCamera::TrackballCamera(const TrackballCamera& other) : Camera(other), polarAngle(other.polarAngle),
	azimuthAngle(other.azimuthAngle), radius(other.radius), trackPosition(other.trackPosition)
{
	updateCartesianCoordinates();
}

TrackballCamera::~TrackballCamera()
{
}

float TrackballCamera::getAzimuthAngle() const
{
	return azimuthAngle;
}

float TrackballCamera::getPolarAngle() const
{
	return polarAngle;
}

float TrackballCamera::getRadius() const
{
	return radius;
}

const vec3& TrackballCamera::getTrackPosition() const
{
	return trackPosition;
}

void TrackballCamera::update(int mouseXFrameOffset, int mouseYFrameOffset)
{
}

vec3 TrackballCamera::sphericalCoordinatesToCartesian() const
{
	float y = cos(polarAngle) * radius;
	float z = sin(polarAngle) * cos(azimuthAngle) * radius;
	float x = sin(polarAngle) * sin(azimuthAngle) * radius;

	return vec3(x,y,z);
}

void TrackballCamera::updateCartesianCoordinates()
{
	position = sphericalCoordinatesToCartesian();
	look = trackPosition - position;
}