#include <nex/particle/Particle.hpp>
#include <glm/gtc/matrix_transform.hpp>

nex::Particle::Particle(const glm::vec3& pos, 
	const glm::vec3& vel, 
	float rotation, 
	float scale, 
	float lifeTime, 
	float gravityInfluence) :

	mPosition(pos), 
	mVelocity(vel), 
	mRotation(rotation), 
	mScale(scale), 
	mLifeTime(lifeTime), 
	mGravityInfluence(gravityInfluence), 
	mElapsedTime(0.0f),
	mIsAlive(true)
{
}

float nex::Particle::getElapsedTime() const
{
	return mElapsedTime;
}

float nex::Particle::getGravityInfluence() const
{
	return mGravityInfluence;
}

float nex::Particle::getLifeTime() const
{
	return mLifeTime;
}

const glm::vec3& nex::Particle::getPosition() const
{
	return mPosition;
}

float nex::Particle::getRotation() const
{
	return mRotation;
}

float nex::Particle::getScale() const
{
	return mScale;
}

const glm::vec3& nex::Particle::getVelocity() const
{
	return mVelocity;
}

bool nex::Particle::isAlive() const
{
	return mIsAlive;
}

void nex::Particle::setGravityInfluence(float influence)
{
	mGravityInfluence = influence;
}

void nex::Particle::setLifeTime(float lifeTime)
{
	mLifeTime = lifeTime;
}

void nex::Particle::setPosition(const glm::vec3& pos)
{
	mPosition = pos;
}

void nex::Particle::setRotation(float rotation)
{
	mRotation = rotation;
}

void nex::Particle::setScale(float scale)
{
	mScale = scale;
}

void nex::Particle::setVelocity(const glm::vec3& vel)
{
	mVelocity = vel;
}

bool nex::Particle::update(const glm::mat4& view, float frameTime)
{	
	mElapsedTime += frameTime;

	mVelocity.y += GRAVITY * mGravityInfluence* frameTime;

	glm::vec3 change = mVelocity * frameTime + mPosition;

	mIsAlive = mElapsedTime < mLifeTime;

	auto model = glm::translate(glm::mat4(), mPosition);
	model[0][0] = view[0][0];
	model[0][1] = view[1][0];
	model[0][2] = view[2][0];
	model[1][0] = view[0][1];
	model[1][1] = view[1][1];
	model[2][0] = view[0][2];
	model[2][1] = view[1][2];
	model[2][2] = view[2][2];



	model = glm::rotate(model, mRotation, glm::vec3(0,0,1));
	model = glm::scale(model, glm::vec3(mScale));

	mModelView = view * model;

	return mIsAlive;
}