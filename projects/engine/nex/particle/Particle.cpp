#include <nex/particle/Particle.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/mesh/MeshManager.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/mesh/Mesh.hpp>
#include <nex/mesh/MeshGroup.hpp>
#include <nex/material/Material.hpp>
#include <nex/renderer/RenderCommand.hpp>
#include <nex/renderer/RenderCommandQueue.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/math/Random.hpp>
#include <functional>

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

const nex::AABB& nex::Particle::getBoundingBox() const
{
	return mBox;
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

const glm::mat4& nex::Particle::getWorldTrafo() const
{
	return mWorldTrafo;
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

	mPosition += mVelocity * frameTime;

	mIsAlive = mElapsedTime < mLifeTime;

	mWorldTrafo = glm::mat4(1.0f);
	

	glm::mat4 invView = transpose(glm::mat3(view));
	invView[3][3] = 1.0f;

	/*mWorldTrafo[0][0] = view[0][0];
	mWorldTrafo[0][1] = view[1][0];
	mWorldTrafo[0][2] = view[2][0];
	mWorldTrafo[1][0] = view[0][1];
	mWorldTrafo[1][1] = view[1][1];
	mWorldTrafo[2][0] = view[0][2];
	mWorldTrafo[2][1] = view[1][2];
	mWorldTrafo[2][2] = view[2][2];*/


	auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(mScale));
	auto rotation = glm::rotate(glm::mat4(1.0f), mRotation, glm::vec3(0,0,1));
	mWorldTrafo[3] = glm::vec4(mPosition, 1.0f);
	mWorldTrafo = mWorldTrafo * invView * rotation * scale;
	return mIsAlive;
}

nex::ParticleShader::ParticleShader() : 
	Shader(nex::ShaderProgram::create("particle/particle_vs.glsl", "particle/particle_fs.glsl"))
{
	mViewProj = { mProgram->getUniformLocation("viewProj"), UniformType::MAT4 };
	mInverseView3x3 = { mProgram->getUniformLocation("invView3x3"), UniformType::MAT3 };
	mModel = { mProgram->getUniformLocation("model"), UniformType::MAT4 };
}

void nex::ParticleShader::updateConstants(const Constants& constants) {
	auto viewProj = constants.camera->getProjectionMatrix() * constants.camera->getView();
	mProgram->setMat4(mViewProj.location, viewProj);
	glm::mat3 invView = constants.camera->getView();
	mProgram->setMat3(mInverseView3x3.location, transpose(invView));
}

void nex::ParticleShader::updateInstance(const glm::mat4& modelMatrix, const glm::mat4& prevModelMatrix) {
	mProgram->setMat4(mModel.location, modelMatrix);
}

nex::ParticleRenderer::ParticleRenderer(const Material* material)
{
	mParticleMB.setReferenceMaterial(material);
	mParticleMB.add(MeshManager::get()->getUnitPlane(), material);
	mParticleMB.calcBoundingBox();

	mPrototype.isBoneAnimated = false;
	mPrototype.batch = &mParticleMB;
	mPrototype.boundingBox = nullptr;
	mPrototype.renderFunc = ParticleRenderer::render;
}

void nex::ParticleRenderer::createRenderCommands(
	const ParticleIterator& begin,
	const ParticleIterator& end,
	const nex::AABB* boundingBox,
	RenderCommandQueue& commandQueue)
{
	/*for (auto it = begin; it != end; ++it) {

		for (auto prototype : mPrototypes) {
			prototype.worldTrafo = &it->getWorldTrafo();
			prototype.prevWorldTrafo = prototype.worldTrafo;
			commandQueue.push(prototype);
		}
	}*/

	mRange.begin = begin;
	mRange.end = end;

	auto command = mPrototype;

	command.data = &mRange;
	command.boundingBox = boundingBox;
	commandQueue.push(command);
}

nex::RenderState nex::ParticleRenderer::createParticleRenderState()
{
	RenderState state;
	state.doBlend = true;
	state.doDepthWrite = false;
	state.doShadowCast = false;
	state.doShadowReceive = false;
	state.doCullFaces = false;

	return state;
}

std::unique_ptr<nex::Material> nex::ParticleRenderer::createParticleMaterial(std::unique_ptr<Material> material)
{
	material->getRenderState() = createParticleRenderState();
	return material;
}

void nex::ParticleRenderer::render(const RenderCommand& command, 
	Shader** lastShaderPtr, 
	const Constants& constants, 
	const ShaderOverride<nex::Shader>& overrides, 
	const RenderState* overwriteState)
{
	auto* lastShader = *lastShaderPtr;
	auto* currentShader = command.batch->getShader();

	if (lastShader != currentShader) {
		*lastShaderPtr = currentShader;

		currentShader->bind();
		currentShader->updateConstants(constants);
	}

	const auto& range = *(const ParticleRange*) command.data;

	for (auto it = range.begin; it != range.end; ++it) {
		
		auto& worldTrafo = it->getWorldTrafo();
		currentShader->updateInstance(worldTrafo, worldTrafo);
		
		for (auto& pair : command.batch->getEntries()) {
			Drawer::draw(currentShader, pair.first, pair.second, overwriteState);
		}
	}
}

nex::ParticleRenderer::~ParticleRenderer() = default;


nex::ParticleManager::ParticleManager(size_t maxParticles) : mLastActive(-1), mParticles(maxParticles)
{
}

void nex::ParticleManager::create(const glm::vec3& pos, 
	const glm::vec3& vel, 
	float rotation, 
	float scale, 
	float lifeTime, 
	float gravityInfluence)
{
	const auto nextSize = mLastActive + 1;
	if (nextSize >= mParticles.size()) return;

	++mLastActive;
	mParticles[mLastActive] = Particle(pos, vel, rotation, scale, lifeTime, gravityInfluence);
}

void nex::ParticleManager::frameUpdate(const glm::mat4& view, float frameTime)
{
	// Update all particles and retrieve the highest index of active particles.
	for (int i = mLastActive; i >= 0; --i) {
		auto isAlive = mParticles[i].update(view, frameTime);
		if (isAlive && i > mLastActive) mLastActive = i;
	}

	// Put all active particles to the front and update the last active index
	for (int i = mLastActive; i >= 0; --i) {
		auto& particle = mParticles[i];

		// swap not active particle with last active particle
		if (!particle.isAlive()) {
			auto& lastActive = mParticles[mLastActive];

			if (mLastActive != i)
				std::swap(particle, lastActive);

			--mLastActive;
		}
	}
}

nex::ParticleIterator nex::ParticleManager::getParticleBegin() const
{
	if (mLastActive < 0) return mParticles.end();
	return mParticles.begin();
}

nex::ParticleIterator nex::ParticleManager::getParticleEnd() const
{
	if (mLastActive < 0) return mParticles.end();
	return mParticles.begin() + mLastActive + 1;
}

nex::ParticleSystem::ParticleSystem(
	AABB boundingBox,
	float gravityInfluence,
	float lifeTime,
	std::unique_ptr<Material> material,
	size_t maxParticles,
	const glm::vec3& position,
	float pps, 
	float rotation,
	float scale,
	float speed) :
	mBox(std::move(boundingBox)),
	mGravityInfluence(gravityInfluence),
	mLifeTime(lifeTime),
	mMaterial(std::move(material)),
	mPosition(position), 
	mPartialParticles(0.0f),
	mPps(pps), 
	mRotation(rotation),
	mScale(scale),
	mSpeed(speed),
	mManager(maxParticles),
	mRenderer(mMaterial.get())
{
}

void nex::ParticleSystem::collectRenderCommands(RenderCommandQueue& commandQueue)
{
	mRenderer.createRenderCommands(mManager.getParticleBegin(), 
		mManager.getParticleEnd(),
		&mBox,
		commandQueue);
}

void nex::ParticleSystem::frameUpdate(const Constants& constants)
{
	const auto& frameTime = constants.frameTime;
	auto particlesToCreate = mPps * frameTime;
	double count1;
	mPartialParticles += std::modf(particlesToCreate, &count1);

	double count2;
	mPartialParticles = std::modf(mPartialParticles, &count2);

	size_t count = static_cast<size_t>(count1 + count2);

	for (size_t i = 0; i < count; ++i) {
		emit(mPosition);
	}

	mManager.frameUpdate(constants.camera->getView(), frameTime);
}

const nex::AABB& nex::ParticleSystem::getBoundingBox() const
{
	return mBox;
}

const glm::vec3& nex::ParticleSystem::getPosition() const
{
	return mPosition;
}

void nex::ParticleSystem::setPosition(const glm::vec3& pos)
{
	mPosition = pos;
}

void nex::ParticleSystem::emit(const glm::vec3& center)
{
	auto dirX = Random::nextFloat() * 2.0f - 1.0f;
	auto dirZ = Random::nextFloat() * 2.0f - 1.0f;
	glm::vec3 velocity = mSpeed * normalize(glm::vec3(dirX, 1.0f, dirZ));
	mManager.create(center, velocity, mRotation, mScale, mLifeTime, mGravityInfluence);
}