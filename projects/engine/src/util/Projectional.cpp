#include <util/Projectional.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <stdexcept>
#include <platform/logging/GlobalLoggingServer.hpp>

using namespace std;
using namespace glm;

Projectional::Projectional(float aspectRatio, float fov, Frustum frustum,
	vec3 look, vec3 position, vec3 up) : aspectRatio(aspectRatio), fov(fov),
	frustum(frustum), logClient(platform::getLogServer()), look(look), position(position), 
	revalidate(true), up(up)
{
	logClient.setPrefix("[Projectional]");
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

const Frustum& Projectional::getFrustum() const
{
	return frustum;
}

FrustumCube Projectional::getFrustumCube()
{
	update();
	FrustumCube cube;
	cube.m_near = getFrustumPlane(frustum.nearPlane);
	cube.m_far = getFrustumPlane(frustum.farPlane);

	return move(cube);
}

FrustumPlane Projectional::getFrustumPlane(float zValue) const
{
	FrustumPlane result;
	vec3& lBot = result.leftBottom;
	vec3& lTop = result.leftBottom;
	vec3& rBot = result.rightBottom;
	vec3& rTop = result.rightTop;

	lBot.x = frustum.left;
	lTop.x = frustum.left;
	rBot.x = frustum.right;
	rTop.x = frustum.right;

	lBot.y = frustum.bottom;
	rBot.y = frustum.bottom;
	lTop.y = frustum.top;
	rTop.y = frustum.top;

	lBot.z = zValue;
	lTop.z = zValue;
	rBot.z = zValue;
	rTop.z = zValue;

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
	default: throw runtime_error("Projectional::getProjection: Unknown projection mode.");
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
	look = location - position;
	revalidate = true;
}

void Projectional::setAspectRatio(float ratio)
{
	if (ratio <= 0)
		throw runtime_error("Projectional::setAspectRatio(float): aspect ratio has to be greater 0!");
	this->aspectRatio = ratio;
}


void Projectional::setFOV(float fov)
{
	this->fov = fov;
	revalidate = true;
}

void Projectional::setFrustum(Frustum frustum)
{
	this->frustum = move(frustum);
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

void Projectional::calcFrustum()
{
	// calculate near plane 
	float x = tan(radians(fov / 2.0f)) * aspectRatio;
	float y = tan(radians(fov / 2.0f)) * 1.0f / aspectRatio;


	frustum.left = -x;
	frustum.right = x;
	frustum.bottom = -y;
	frustum.top = y;
}

void Projectional::update()
{
	// only update if changes have occurred
	if (revalidate)
	{
		//calcFrustum();
		float fovRad = radians(fov);
		orthographic = ortho(fovRad * frustum.left, fovRad * frustum.right, fovRad * frustum.bottom,
			fovRad * frustum.top, frustum.nearPlane, frustum.farPlane);
		perspective = glm::perspective(radians(fov),
			aspectRatio, frustum.nearPlane, frustum.farPlane);
		calcView();
		revalidate = false;
	}
}