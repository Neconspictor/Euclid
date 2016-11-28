#include <camera/TrackballQuatCamera.hpp>
#include <platform/Input.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.inl>

using namespace std;
using namespace glm;

TrackballQuatCamera::TrackballQuatCamera(Window* window) : Camera(window), 
	prevMouseX(0), prevMouseY(0), halfScreenWidth(0), halfScreenHeight(0)
{
	radius = 1;
	up = {0,1,0};
	orientation = quat(1,0,0,0);
	prevOrientation = quat(1, 0, 0, 0);
	rotatedPos = { 0,0,3*radius};
	fov = 45;
	Renderer::Viewport viewport = window->getViewport();
	prevMouseX = viewport.width / 2;
	prevMouseY = viewport.height / 2;
}

TrackballQuatCamera::TrackballQuatCamera(Window* window, vec3 trackPosition, float radius, vec3 up) :
	TrackballQuatCamera(window)
{
	this->trackPosition = trackPosition;
	this->radius = radius;
	orientation = quat(1, 0, 0, 0);
	this->up = up;
}

TrackballQuatCamera::TrackballQuatCamera(const TrackballQuatCamera& other) : Camera(other.window), radius(other.radius),
	trackPosition(other.trackPosition), prevMouseX(other.prevMouseX), prevMouseY(other.prevMouseY), 
	halfScreenWidth(other.halfScreenWidth), halfScreenHeight(other.halfScreenHeight), orientation(other.orientation)
{
}

TrackballQuatCamera::~TrackballQuatCamera()
{
}

void TrackballQuatCamera::calcView()
{
	Camera::calcView();
	view =  view * getArcBallTransformation();
}

mat4 TrackballQuatCamera::getArcBallTransformation()
{
	mat4 result  = mat4_cast(orientation);
	//view = transpose(view);
	//result = transpose(result);
	//position = orientation * vec3{ 0,0, -3.0f*radius };
	//result = translate(result, position);
	return result;
}

float TrackballQuatCamera::getRadius() const
{
	return radius;
}

const quat& TrackballQuatCamera::getOrientation() const
{
	return orientation;
}

const vec3& TrackballQuatCamera::getTrackPosition() const
{
	return trackPosition;
}

quat TrackballQuatCamera::multiply(quat q1, quat q2)
{
	vec3 v1(q1.x, q1.y, q1.z);
	vec3 v2(q2.x, q2.y, q2.z);

	vec3 crossP = cross(v1, v2);                   // v x v'
	float dotP = dot(v1, v2);                         // v . v'
	vec3 v3 = crossP + (q1.w * v2) + (q2.w * v1);   // v x v' + sv' + s'v

	return quat(q1.w * q2.w - dotP, v3.x, v3.y, v3.z);
}

void TrackballQuatCamera::setHalfScreenWidth(float width)
{
	halfScreenWidth = width;
}

void TrackballQuatCamera::setHalfScreenHeight(float height)
{
	halfScreenHeight = height;
}

void TrackballQuatCamera::setRadius(float radius)
{
	this->radius = radius;
}

void TrackballQuatCamera::setTrackPosition(vec3 trackPosition)
{
	this->trackPosition = trackPosition;
}

void TrackballQuatCamera::setUpDirection(const vec3& up)
{
	this->up = up;
}

void TrackballQuatCamera::update(int mouseXFrameOffset, int mouseYFrameOffset)
{
	Renderer::Viewport viewport = window->getViewport();
	updateOnResize(viewport.width, viewport.height);

	if (window->getInputDevice()->isPressed(Input::LeftMouseButton))
	{
		prevMouseX = static_cast<float>(mouseXFrameOffset);
		prevMouseY = static_cast<float>(mouseYFrameOffset);
		prevOrientation = orientation;
		return;
	}


	if (!window->getInputDevice()->isDown(Input::LeftMouseButton))
	{
		return;
	}

	vec3 currentSphereVec = getUnitVector(static_cast<float>(mouseXFrameOffset), static_cast<float>(mouseYFrameOffset));
	vec3 previousSphereVec = getUnitVector(prevMouseX, prevMouseY);

	quat rotation = getRotation(previousSphereVec, currentSphereVec);
	orientation = multiply(rotation, prevOrientation);
}

void TrackballQuatCamera::updateOnResize(int screenWidth, int screenHeight)
{
	halfScreenWidth = screenWidth / 2;
	halfScreenHeight = screenHeight / 2;
}

bool TrackballQuatCamera::equal(const vec3& v1, const vec3& v2, float epsilon)
{
	auto eps = epsilonEqual(v1, v2, epsilon);
	return eps.x && eps.y && eps.z;
}

quat TrackballQuatCamera::createRotationQuat(vec3 axis, float angle)
{
	axis = normalize(axis);                  // convert to unit vector
	float sine = sinf(angle);       // angle is radian
	float w = cosf(angle);
	float x = axis.x * sine;
	float y = axis.y * sine;
	float z = axis.z * sine;
	return quat(w, x, y, z);
}

quat TrackballQuatCamera::getRotation( const vec3& from, const vec3& to)
{
	static const float EPSILON = 0.001f;
	static const float HALF_PI = 1.570796f;

	// if two vectors are equal return the vector with 0 rotation
	if (equal(from, to, EPSILON))
	{
		return createRotationQuat(from, 0);
	}

	// if two vectors are opposite return a perpendicular vector with 180 angle
	if (equal(from, -to, EPSILON))
	{
		vec3 axis;
		if (from.x > -EPSILON && from.x < EPSILON)       // if x ~= 0
			axis = {1, 0, 0};
		else if (from.y > -EPSILON && from.y < EPSILON)  // if y ~= 0
			axis = { 0, 1, 0 };
		else                                        // if z ~= 0
			axis = { 0, 0, 1 };
		return createRotationQuat(axis, HALF_PI);
	}


	vec3 copyFrom = normalize(from);
	vec3 copyTo = normalize(to);

	vec3 axis = cross(copyFrom, copyTo);           // compute rotation axis
	float angle = acosf(dot(copyFrom, copyTo));    // rotation angle

	return createRotationQuat(axis, 0.5f*angle);   // return half angle for quaternion*/
}

vec3 TrackballQuatCamera::getUnitVector(float x, float y)
{
	vec3 vec = getVector(x, y);
	vec = normalize(vec);
	return vec;
}

vec3 TrackballQuatCamera::getVector(float x, float y)
{
	if (radius == 0 || halfScreenWidth == 0 || halfScreenHeight == 0)
		return vec3(0, 0, 0);

	// compute mouse position from the centre of screen (-half ~ +half)
	float halfScreenX = (x - halfScreenWidth)/ 400.0f;
	float halfScreenY = (halfScreenHeight - y)/ 400.0f;    //bottom values are higher than top values -> revers order

	return getVectorWithArc(halfScreenX, halfScreenY); // default mode
}

vec3 TrackballQuatCamera::getVectorWithArc(float x, float y)
{
	float arc = sqrtf(x*x + y*y);   // legnth between cursor and screen center
	float a = arc / radius;         // arc = r * a
	float b = atan2f(y, x);         // angle on x-y plane
	float x2 = radius * sinf(a);    // x rotated by "a" on x-z plane

	vec3 vec;
	vec.x = x2 * cosf(b);
	vec.y = x2 * sinf(b);
	vec.z = radius * cosf(a);
	return vec;
}

string TrackballQuatCamera::toString(const vec3& vec) const
{
	stringstream ss;
	ss << vec.x << ", " << vec.y << ", " << vec.z;
	return ss.str();
}

string TrackballQuatCamera::toString(const quat& quaternion) const
{
	stringstream ss;
	ss << quaternion.x << ", " << quaternion.y << ", " << quaternion.z << ", " << quaternion.w;
	return ss.str();
}
