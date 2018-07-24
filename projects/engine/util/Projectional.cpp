#include <util/Projectional.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <util/Math.hpp>

using namespace std;
using namespace glm;

Projectional::Projectional(float aspectRatio, float fov, float perspNear, float perspFar, Frustum frustum,
	vec3 look, vec3 position, vec3 up) : aspectRatio(aspectRatio), fov(fov),
	logClient(platform::getLogServer()), look(look), orthoFrustum(frustum), position(position), 
	revalidate(true), up(up)
{
	logClient.setPrefix("[Projectional]");
	perspFrustum.nearPlane = perspNear;
	perspFrustum.farPlane = perspFar;
}

Projectional::~Projectional()
{
}

void Projectional::calcView()
{
	view = glm::lookAt(
		position,
		position + look,
		up
		);
}

float Projectional::getAspectRatio() const
{
	return aspectRatio;
}

const vec3& Projectional::getLook() const
{
	return look;
}

float Projectional::getFOV() const
{
	return fov;
}

const Frustum& Projectional::getFrustum(ProjectionMode mode)
{
	update();
	if (mode == Orthographic)
		 return orthoFrustum;
	else
		return perspFrustum;
}

FrustumCuboid Projectional::getFrustumCuboid(ProjectionMode mode, float zStart, float zEnd)
{
	assert(isInRange(zStart, 0.0f, 1.0f));
	assert(isInRange(zEnd, 0.0f, 1.0f));
	
	FrustumCuboid cube;
	cube.m_near = getFrustumPlane(mode, zStart);
	cube.m_far = getFrustumPlane(mode, zEnd);

	return move(cube);
}

FrustumPlane Projectional::getFrustumPlane(ProjectionMode mode, float zValue)
{
	update();
	FrustumPlane result;
	float l, r, t, b, n, f, z;

   #ifndef USE_LEFT_HANDED_COORDINATE_SYSTEM
	zValue *= -1; // the z-axis is inverted on right handed systems
   #endif

	switch (mode)
	{
	case Orthographic: {
		l = orthoFrustum.left;
		r = orthoFrustum.right;
		t = orthoFrustum.top;
		b = orthoFrustum.bottom;
		n = orthoFrustum.nearPlane;
		f = orthoFrustum.farPlane;
		z = zValue*(f - n) + n;
		break;
	}
	case Perspective: {
		n = perspFrustum.nearPlane;
		f = perspFrustum.farPlane;
		z = zValue*(f - n) + n;
		l = perspFrustum.left * z;
		r = perspFrustum.right * z;
		t = perspFrustum.top * z;
		b = perspFrustum.bottom * z;
		break;
	}
	default: throw runtime_error("Projectional::getFrustumPlane(): Unknown projection mode: " + to_string(mode));
	}

	result.leftBottom = {l,b,z};
	result.leftTop = {l,t,z};
	result.rightBottom = {r,b,z};
	result.rightTop = {r,t,z};

	return move(result);
}

const mat4& Projectional::getOrthoProjection()
{
	update();
	return orthographic;
}

const mat4& Projectional::getPerspProjection()
{
	update();
	return perspective;
}

const mat4& Projectional::getProjection(ProjectionMode mode)
{
	switch(mode)
	{
	case ProjectionMode::Orthographic: return getOrthoProjection();
	case ProjectionMode::Perspective: return getPerspProjection();
	default: throw runtime_error("Projectional::getProjection: Unknown projection mode: " + to_string(mode));
	}
}

const vec3& Projectional::getPosition() const
{
	return position;
}

const vec3& Projectional::getUp() const
{
	return up;
}

const mat4& Projectional::getView()
{
	update();
	return view;
}

void Projectional::lookAt(vec3 location)
{
	//setLook(location - position);
	look = normalize(location - position);
	revalidate = true;
}

void Projectional::setAspectRatio(float ratio)
{
	if (ratio <= 0)
		throw runtime_error("Projectional::setAspectRatio(float): aspect ratio has to be greater 0!");
	this->aspectRatio = ratio;
	revalidate = true;
}


void Projectional::setFOV(float fov)
{
	this->fov = fov;
	revalidate = true;
}

void Projectional::setOrthoFrustum(Frustum frustum)
{
	orthoFrustum = move(frustum);
	revalidate = true;
}

void Projectional::setLook(vec3 look)
{
	look = normalize(look);
	if (isnan(look.x) ||
		isnan(look.y) ||
		isnan(look.z))
	{
		throw runtime_error("Projectional::setDirection(glm::vec3): specified a non valid direction vector!");
	}

	// no we can savely change the object state!
	this->look = look;
	revalidate = true;
}

void Projectional::setPosition(vec3 position)
{
	this->position = move(position);
	revalidate = true;
}

void Projectional::setUp(vec3 up)
{
	this->up = move(up);
	revalidate = true;
}

void Projectional::calcPerspFrustum()
{
	// calculate near plane 
	float x = tan(radians(fov * aspectRatio / 2.0f));
	float y = tan(radians(fov / 2.0f));


	perspFrustum.left = -x;
	perspFrustum.right = x;
	perspFrustum.bottom = -y;
	perspFrustum.top = y;
}

void Projectional::update()
{
	// only update if changes have occurred
	if (revalidate)
	{
		orthographic = ortho(orthoFrustum.left, orthoFrustum.right, 
			orthoFrustum.bottom, orthoFrustum.top, 
			orthoFrustum.nearPlane, orthoFrustum.farPlane);
		perspective = glm::perspective(radians(fov),
			aspectRatio, perspFrustum.nearPlane, perspFrustum.farPlane);
		calcView();
		calcPerspFrustum();
		revalidate = false;
	}
}