#include <nex/camera/TrackballQuatCamera.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <sstream>

using namespace std;
using namespace glm;

nex::TrackballQuatCamera::TrackballQuatCamera()
{
	trackPosition = { 0.0f, 0.0f, 0 };
	radius = 3; 
	look = { 0, 0, -1 };
	up = { 0,1,0 };
	mCurrentPosition = trackPosition - radius * look;
	mTargetPosition = mCurrentPosition;
	Camera(mCurrentPosition, look, up);
	init();
}

nex::TrackballQuatCamera::TrackballQuatCamera(glm::vec3 trackPosition, float radius, glm::vec3 look, glm::vec3 up)
{
	mCurrentPosition = trackPosition - radius*look;
	mTargetPosition = mCurrentPosition;
	Camera(mCurrentPosition, look, up);
	init();
	this->trackPosition = trackPosition;
	this->radius = radius;
}

nex::TrackballQuatCamera::TrackballQuatCamera(const TrackballQuatCamera& other) : Camera(other), radius(other.radius),
	trackPosition(other.trackPosition), prevMouseX(other.prevMouseX), prevMouseY(other.prevMouseY), 
	halfScreenWidth(other.halfScreenWidth), halfScreenHeight(other.halfScreenHeight), orientation(other.orientation)
{
}


void nex::TrackballQuatCamera::calcView()
{
	view = glm::lookAt(mCurrentPosition, trackPosition, up);
	look = glm::normalize(mCurrentPosition - trackPosition);
	glm::mat4 trackTargetTrans;
	glm::mat4 trackTargetTransInverse;
	trackTargetTrans = translate(trackTargetTrans, -trackPosition);
	trackTargetTransInverse = translate(trackTargetTransInverse, trackPosition);
	view = view * trackTargetTransInverse * mat4_cast(orientation) * trackTargetTrans;
	//view =  view * getArcBallTransformation();
}

glm::mat4 nex::TrackballQuatCamera::getArcBallTransformation()
{
	glm::mat4 result  = mat4_cast(orientation);
	//view = transpose(view);
	//result = transpose(result);
	//position = orientation * vec3{ 0,0, -3.0f*radius };
	//result = translate(result, position);
	return result;
}

float nex::TrackballQuatCamera::getRadius() const
{
	return radius;
}

const glm::quat& nex::TrackballQuatCamera::getOrientation() const
{
	return orientation;
}

const glm::vec3& nex::TrackballQuatCamera::getTrackPosition() const
{
	return trackPosition;
}

glm::quat nex::TrackballQuatCamera::multiply(glm::quat q1, glm::quat q2)
{
	glm::vec3 v1(q1.x, q1.y, q1.z);
	glm::vec3 v2(q2.x, q2.y, q2.z);

	glm::vec3 crossP = cross(v1, v2);                   // v x v'
	float dotP = dot(v1, v2);                         // v . v'
	glm::vec3 v3 = crossP + (q1.w * v2) + (q2.w * v1);   // v x v' + sv' + s'v

	return glm::quat(q1.w * q2.w - dotP, v3.x, v3.y, v3.z);
}

void nex::TrackballQuatCamera::setHalfScreenWidth(int width)
{
	halfScreenWidth = width;
}

void nex::TrackballQuatCamera::setHalfScreenHeight(int height)
{
	halfScreenHeight = height;
}

void nex::TrackballQuatCamera::setRadius(float radius)
{
	this->radius = radius;
}

void nex::TrackballQuatCamera::setTrackPosition(glm::vec3 trackPosition)
{
	this->trackPosition = trackPosition;
}

void nex::TrackballQuatCamera::update(Input* input, float frameTime)
{
	MouseOffset data = input->getFrameMouseOffset();
	float xPos = static_cast<float>(data.xAbsolute);
	float yPos = static_cast<float>(data.yAbsolute);
	if (input->isPressed(Input::LeftMouseButton))
	{
		prevMouseX = xPos;
		prevMouseY = yPos;
		prevOrientation = orientation;
		return;
	}


	if (!input->isDown(Input::LeftMouseButton))
	{
		return;
	}

	glm::vec3 currentSphereVec = getUnitVector(xPos, yPos);
	glm::vec3 previousSphereVec = getUnitVector(prevMouseX, prevMouseY);

	glm::quat rotation = getRotation(previousSphereVec, currentSphereVec);
	orientation = multiply(rotation, prevOrientation);
}

void nex::TrackballQuatCamera::updateOnResize(unsigned screenWidth, unsigned screenHeight)
{
	halfScreenWidth = screenWidth / 2;
	halfScreenHeight = screenHeight / 2;
}

bool nex::TrackballQuatCamera::equal(const glm::vec3& v1, const glm::vec3& v2, float epsilon)
{
	auto eps = glm::epsilonEqual(v1, v2, epsilon);
	return eps.x && eps.y && eps.z;
}

glm::quat nex::TrackballQuatCamera::createRotationQuat(glm::vec3 axis, float angle)
{
	axis = glm::normalize(axis);                  // convert to unit vector
	float sine = sinf(angle);       // angle is radian
	float w = cosf(angle);
	float x = axis.x * sine;
	float y = axis.y * sine;
	float z = axis.z * sine;
	return glm::quat(w, x, y, z);
}

glm::quat nex::TrackballQuatCamera::getRotation( const glm::vec3& from, const glm::vec3& to)
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
		glm::vec3 axis;
		if (from.x > -EPSILON && from.x < EPSILON)       // if x ~= 0
			axis = {1, 0, 0};
		else if (from.y > -EPSILON && from.y < EPSILON)  // if y ~= 0
			axis = { 0, 1, 0 };
		else                                        // if z ~= 0
			axis = { 0, 0, 1 };
		return createRotationQuat(axis, HALF_PI);
	}


	glm::vec3 copyFrom = normalize(from);
	glm::vec3 copyTo = normalize(to);

	glm::vec3 axis = glm::cross(copyFrom, copyTo);           // compute rotation axis
	float angle = acosf(glm::dot(copyFrom, copyTo));    // rotation angle

	return createRotationQuat(axis, 0.5f*angle);   // return half angle for quaternion*/
}

glm::vec3 nex::TrackballQuatCamera::getUnitVector(float x, float y)
{
	glm::vec3 vec = getVector(x, y);
	vec = glm::normalize(vec);
	return vec;
}

glm::vec3 nex::TrackballQuatCamera::getVector(float x, float y)
{
	if (radius == 0 || halfScreenWidth == 0 || halfScreenHeight == 0)
		return glm::vec3(0, 0, 0);

	// compute mouse position from the centre of screen (-half ~ +half)
	float halfScreenX = (x - halfScreenWidth)/ (float)halfScreenWidth;
	float halfScreenY = (halfScreenHeight - y)/ (float)halfScreenHeight;    //bottom values are higher than top values -> revers order

	return getVectorWithArc(halfScreenX, halfScreenY); // default mode
}

glm::vec3 nex::TrackballQuatCamera::getVectorWithArc(float x, float y)
{
	float arc = sqrtf(x*x + y*y);   // legnth between cursor and screen center
	float a = arc;// / radius;         // arc = r * a
	float b = atan2f(y, x);         // angle on x-y plane
	float x2 = /*radius **/ sinf(a);    // x rotated by "a" on x-z plane

	glm::vec3 vec;
	vec.x = x2 * cosf(b);
	vec.y = x2 * sinf(b);
	vec.z = /*radius **/ cosf(a);
	return vec;
}

void nex::TrackballQuatCamera::init()
{
	prevMouseX = 0; 
	prevMouseY = 0; 
	halfScreenWidth = 0; 
	halfScreenHeight = 0;
	orientation = glm::quat(1, 0, 0, 0);
	prevOrientation = glm::quat(1, 0, 0, 0);
	fov = 45;
}

string nex::TrackballQuatCamera::toString(const glm::vec3& vec) const
{
	stringstream ss;
	ss << vec.x << ", " << vec.y << ", " << vec.z;
	return ss.str();
}

string nex::TrackballQuatCamera::toString(const glm::quat& quaternion) const
{
	stringstream ss;
	ss << quaternion.x << ", " << quaternion.y << ", " << quaternion.z << ", " << quaternion.w;
	return ss.str();
}