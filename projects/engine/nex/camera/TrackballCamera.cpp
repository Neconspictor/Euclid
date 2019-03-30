#ifndef _USE_MATH_DEFINES
	#define _USE_MATH_DEFINES
#endif

#include <cmath>
#include <nex/camera/TrackballCamera.hpp>
#include <glm/glm.hpp>
#include <math.h>
#include <nex/util/ExceptionHandling.hpp>


using namespace std;
using namespace glm;

nex::TrackballCamera::TrackballCamera() : Camera(), coords({ 0,0,100 }), trackPosition(0,0,0)
{
	updateCartesianCoordinates();
}

nex::TrackballCamera::TrackballCamera(vec3 trackPosition, float polarAngle, float azimuthAngle, float radius) : Camera(),
coords({ polarAngle, azimuthAngle, radius }), trackPosition(trackPosition)
{
	updateCartesianCoordinates();
}

nex::TrackballCamera::TrackballCamera(const TrackballCamera& other) : Camera(other),
	coords(other.coords), trackPosition(other.trackPosition)
{
	updateCartesianCoordinates();
}

float nex::TrackballCamera::getAzimuthAngle() const
{
	return coords.azimuth;
}

float nex::TrackballCamera::getPolarAngle() const
{
	return coords.polar;
}

float nex::TrackballCamera::getRadius() const
{
	return coords.radius;
}

const vec3& nex::TrackballCamera::getTrackPosition() const
{
	return trackPosition;
}

void nex::TrackballCamera::pan(float xPos, float zPos)
{
	setPosition({xPos, mCurrentPosition.y, zPos});
}

void nex::TrackballCamera::rotate(float polar, float azimuth)
{
	updatePolar(&coords.polar, coords.polar + polar);
	updateCartesianCoordinates();
	updateAzimuth(&coords.azimuth, coords.azimuth + azimuth);
	updateCartesianCoordinates();
}

void nex::TrackballCamera::setAzimuthAngle(float azimuth)
{
	updateAzimuth(&coords.azimuth, azimuth);
	updateCartesianCoordinates();
}

void nex::TrackballCamera::setLook(vec3 direction)
{
	//Do nothing!
}

void nex::TrackballCamera::setPolarAngle(float polar)
{
	updatePolar(&coords.polar, polar);
	updateCartesianCoordinates();
}

void nex::TrackballCamera::setPosition(vec3 position)
{
	mCurrentPosition = position;
	mTargetPosition = mCurrentPosition;
	updateSphericalCoords();
	// update look direction
	look = trackPosition - position;
	revalidate = true;
}

void nex::TrackballCamera::setRadius(float radius)
{
	if (radius < 0) throw_with_trace(runtime_error("TrackballCamera::setRadius(float radius): radius is supposed to be greater/equal zero!"));
	coords.radius = radius;
}

void nex::TrackballCamera::setTrackPosition(vec3 trackPosition)
{
	this->trackPosition = move(trackPosition);
}

void nex::TrackballCamera::setUp(vec3 up)
{
	//Do nothing!
}

void nex::TrackballCamera::update(Input* input, float frameTime)
{
	MouseOffset data = input->getFrameMouseOffset();
	
	// early exit
	if (data.xOffset == 0 && data.yOffset == 0) return;
	float sensitivity = 0.005f;
	float rotationPolar = data.yOffset * sensitivity;
	float rotationAzimuth = data.xOffset * sensitivity;

	LOG(m_logger, nex::Debug) << "coords before: " << degrees(coords.polar) << ", " << degrees(coords.azimuth) << ", " << coords.radius;
	rotate(rotationPolar, rotationAzimuth);
	LOG(m_logger, nex::Debug) << "coords after: " << degrees(coords.polar) << ", " << degrees(coords.azimuth) << ", " << coords.radius;
}

nex::TrackballCamera::SphericalCoord nex::TrackballCamera::cartesianToSpherical(const vec3& position, const vec3& lookTarget) const
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
		LOG(m_logger, nex::Debug) << "Radius is nan!" << endl;
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
		LOG(m_logger, nex::Debug) << "Polar is nan!" << endl;
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
		LOG(m_logger, nex::Debug) << "Azimuth is nan!" << endl;
	}

	if (angleIsNegative) azimuth *= -1;
	return { polar, azimuth, radius };
}

vec3 nex::TrackballCamera::sphericalToCartesian(SphericalCoord sphericalCoord, const vec3& lookTarget) const
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
		LOG(m_logger, nex::Debug) << "TrackballCamera::sphericalToCartesian(): " << x << ", " << y << ", " << z << endl;
	}

	return vec3(x, y, z);
}

void nex::TrackballCamera::updateAzimuth(float* sourceAngle, float newValue) const
{
	*sourceAngle = newValue;
	*sourceAngle = fmod(*sourceAngle, static_cast<float>(2*M_PI));

	if (std::isnan(*sourceAngle))
	{
		LOG(m_logger, nex::Error) << "TrackballCamera::updateAzimuth(): sourceAngle is nan!; newValue: " << newValue << endl;
		*sourceAngle = 0;
	}
}

void nex::TrackballCamera::updatePolar(float* sourceAngle, float newValue) const
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
		LOG(m_logger, nex::Error) << "TrackballCamera::updatePolar(): sourceAngle is nan!; newValue: " << newValue << endl;
		*sourceAngle = 0;
	}
}

void nex::TrackballCamera::updateCartesianCoordinates()
{
	mCurrentPosition = sphericalToCartesian(coords, trackPosition);
	look = trackPosition - mCurrentPosition;
}

void nex::TrackballCamera::updateSphericalCoords()
{
	coords = cartesianToSpherical(mCurrentPosition, move(trackPosition));
	look = trackPosition - mCurrentPosition;
}