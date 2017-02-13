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
	setLook(location - position);
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

void Projectional::update()
{
	// only update if changes have occurred
	if (revalidate)
	{
		orthographic = ortho(frustum.left, frustum.right, frustum.bottom,
			frustum.top, frustum.nearPlane, frustum.farPlane);
		perspective = glm::perspective(radians(fov),
			aspectRatio, frustum.nearPlane, frustum.farPlane);
		calcView();
		revalidate = false;
	}
}