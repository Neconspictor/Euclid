#define _USE_MATH_DEFINES
#include <cmath>
#include <camera/TrackballCamera.hpp>
#include <glm/glm.hpp>


using namespace std;
using namespace glm;

TrackballCamera::TrackballCamera() : Camera(), coords({ 0,0,100 }), trackPosition(0,0,0)
{
	updateCartesianCoordinates();
}

TrackballCamera::TrackballCamera(vec3 trackPosition, float polarAngle, float azimuthAngle, float radius) : Camera(),
coords({ polarAngle, azimuthAngle, radius }), trackPosition(trackPosition)
{
	updateCartesianCoordinates();
}

TrackballCamera::TrackballCamera(const TrackballCamera& other) : Camera(other), 
	coords(other.coords), trackPosition(other.trackPosition)
{
	updateCartesianCoordinates();
}

TrackballCamera::~TrackballCamera()
{
}

float TrackballCamera::getAzimuthAngle() const
{
	return coords.azimuth;
}

float TrackballCamera::getPolarAngle() const
{
	return coords.polar;
}

float TrackballCamera::getRadius() const
{
	return coords.radius;
}

const vec3& TrackballCamera::getTrackPosition() const
{
	return trackPosition;
}

void TrackballCamera::pan(float xPos, float zPos)
{
	setPosition({xPos, position.y, zPos});
}

void TrackballCamera::rotate(float polar, float azimuth)
{
	updatePolar(&coords.polar, coords.polar + polar);
	updateCartesianCoordinates();
	updateAzimuth(&coords.azimuth, coords.azimuth + azimuth);
	updateCartesianCoordinates();
}

void TrackballCamera::setAzimuthAngle(float azimuth)
{
	updateAzimuth(&coords.azimuth, azimuth);
	updateCartesianCoordinates();
}

void TrackballCamera::setLookDirection(const vec3& direction)
{
	//Do nothing!
}

void TrackballCamera::setPolarAngle(float polar)
{
	updatePolar(&coords.polar, polar);
	updateCartesianCoordinates();
}

void TrackballCamera::setPosition(const vec3& position)
{
	this->position = position;
	updateSphericalCoords();
	// update look direction
	look = trackPosition - position;
}

void TrackballCamera::setRadius(float radius)
{
	if (radius < 0) throw runtime_error("TrackballCamera::setRadius(float radius): radius is supposed to be greater/equal zero!");
	coords.radius = radius;
}

void TrackballCamera::setTrackPosition(vec3 trackPosition)
{
	this->trackPosition = move(trackPosition);
}

void TrackballCamera::setUpDirection(const vec3& up)
{
	//Do nothing!
}

void TrackballCamera::update(Input* input, float frameTime)
{
	MouseOffset data = input->getFrameMouseOffset();
	
	// early exit
	if (data.xOffset == 0 && data.yOffset == 0) return;
	float sensitivity = 0.005f;
	float rotationPolar = data.yOffset * sensitivity;
	float rotationAzimuth = data.xOffset * sensitivity;

	LOG(logClient, platform::Debug) << "coords before: " << degrees(coords.polar) << ", " << degrees(coords.azimuth) << ", " << coords.radius;
	rotate(rotationPolar, rotationAzimuth);
	LOG(logClient, platform::Debug) << "coords after: " << degrees(coords.polar) << ", " << degrees(coords.azimuth) << ", " << coords.radius;
}

TrackballCamera::SphericalCoord TrackballCamera::cartesianToSpherical(vec3 position, vec3 lookTarget) const
{
	// Spherical coordinates are always relative to the look target, so shift the position relative to the origin.
	// This step simplifies the calculation of later arccosines.
	vec3 posRelative = position - lookTarget;
	auto& x = posRelative.x;
	auto& y = posRelative.y;
	auto& z = posRelative.z;

	// The radius is now the distance from the relative position to the origin.
	float radius = length(posRelative);

	if (std::isnan(radius))
	{
		LOG(logClient, platform::Debug) << "Radius is nan!" << endl;
	}

	// Spot whether the cartesian position is on the upper or lower half sphere 
	// in relation to the y-z plane with positive x axis as the upper half sphere.
	// this is important for getting the sign of the azimuth angle, as the arccosine
	// provides only angles in [0, pi]. For spotting angles from [-pi,0[ we need 
	// to know if the sign of the angle is negative.
	 bool angleIsNegative = x < 0;

	// y = cos(polar) * radius
	// <=> y/radius = cos(polar)
	// <=> polar = acos(y/radius)

	 float polar = 0;
	 if (radius != 0)
		polar = acos(y / radius);

	if (std::isnan(polar))
	{
		LOG(logClient, platform::Debug) << "Polar is nan!" << endl;
	}

	// z = sin(polar) * cos(azimuth) * radius
	// <=> z/ radius = sin(polar) * cos(azimuth)
	// <=> z / (radius * sin(polar)) = cos(azimuth)
	// <=> azimuth = acos (z / (radius * sin(polar)))
	float azimuth = 0;
	if (radius != 0 && sin(polar) != 0)
	{
		azimuth = acos(z / (radius* sin(polar)));
	}
	if (std::isnan(azimuth))
	{
		LOG(logClient, platform::Debug) << "Azimuth is nan!" << endl;
	}

	if (angleIsNegative) azimuth *= -1;
	return { polar, azimuth, radius };
}

vec3 TrackballCamera::sphericalToCartesian(SphericalCoord sphericalCoord, vec3 lookTarget) const
{
	const float& polar = sphericalCoord.polar;
	const float& azimuth = sphericalCoord.azimuth;
	const float& radius = sphericalCoord.radius;

	// The positive y-axis is the zenith direction, as it is assumed that this axis is also
	// the 'Up' direction on the monitor.
	float y = cos(polar) * radius + lookTarget.y;

	// The z-axis is assumed to be the look direction of the camera. So we use it as the reference
	// direction for the default camera position, i.d. polar and azimuth are both 0.
	float z = sin(polar) * cos(azimuth) * radius + lookTarget.z;

	float x = sin(polar) * sin(azimuth) * radius + lookTarget.x;

	if (polar == 0 && azimuth == 0)
	{
		LOG(logClient, platform::Debug) << "TrackballCamera::sphericalToCartesian(): " << x << ", " << y << ", " << z << endl;
	}

	return vec3(x, y, z);
}

void TrackballCamera::updateAzimuth(float* sourceAngle, float newValue) const
{
	*sourceAngle = newValue;
	*sourceAngle = fmod(*sourceAngle, static_cast<float>(2*M_PI));

	if (std::isnan(*sourceAngle))
	{
		LOG(logClient, platform::Error) << "TrackballCamera::updateAzimuth(): sourceAngle is nan!; newValue: " << newValue << endl;
		*sourceAngle = 0;
	}
}

void TrackballCamera::updatePolar(float* sourceAngle, float newValue) const
{
	*sourceAngle = newValue;

	if (*sourceAngle > M_PI - 0.01)
	{
		*sourceAngle = static_cast<float>(M_PI - 0.01);
	}

	if (*sourceAngle <= 0.01f)
	{
		*sourceAngle = 0.01f;
	}

	if (std::isnan(*sourceAngle))
	{
		LOG(logClient, platform::Error) << "TrackballCamera::updatePolar(): sourceAngle is nan!; newValue: " << newValue << endl;
		*sourceAngle = 0;
	}
}

void TrackballCamera::updateCartesianCoordinates()
{
	position = sphericalToCartesian(move(coords), move(trackPosition));
	look = trackPosition - position;
}

void TrackballCamera::updateSphericalCoords()
{
	coords = cartesianToSpherical(move(position), move(trackPosition));
	look = trackPosition - position;
}