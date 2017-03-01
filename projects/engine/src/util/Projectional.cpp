#include <util/Projectional.hpp>
#include <glm/gtc/matrix_transform.inl>
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

FrustumCuboid Projectional::getFrustumCuboid(ProjectionMode mode)
{
	FrustumCuboid cube;
	cube.m_near = getFrustumPlane(mode, -1);
	cube.m_far = getFrustumPlane(mode, 1);

	return move(cube);
}

FrustumPlane Projectional::getFrustumPlane(ProjectionMode mode, float zValue)
{
	update();
	FrustumPlane result;
	/*vec3& lBot = result.leftBottom;
	vec3& lTop = result.leftTop;
	vec3& rBot = result.rightBottom;
	vec3& rTop = result.rightTop;

	const Frustum* frustum;
	if (mode == Orthographic)
		frustum = &orthoFrustum;
	else
		frustum = &perspFrustum;

	lBot.x = frustum->left;
	lTop.x = frustum->left;
	rBot.x = frustum->right;
	rTop.x = frustum->right;

	lBot.y = frustum->bottom;
	rBot.y = frustum->bottom;
	lTop.y = frustum->top;
	rTop.y = frustum->top;

	lBot.z = zValue;
	lTop.z = zValue;
	rBot.z = zValue;
	rTop.z = zValue;*/

	mat4 inverse;
	if (mode == Orthographic)
		inverse = glm::inverse(orthographic);
	else
		inverse = glm::inverse(perspective);

	result.leftBottom = clippingSpaceToCameraSpace(vec3(-1, -1, zValue), inverse);
	result.leftTop = clippingSpaceToCameraSpace(vec3(-1, 1, zValue), inverse);
	result.rightBottom = clippingSpaceToCameraSpace(vec3(1, -1, zValue), inverse);
	result.rightTop = clippingSpaceToCameraSpace(vec3(1, 1, zValue), inverse);

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
	float x = tan(radians(fov / 2.0f)) * aspectRatio;
	float y = tan(radians(fov / 2.0f)) * 1.0f / aspectRatio;


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
		orthographic = mat4();
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