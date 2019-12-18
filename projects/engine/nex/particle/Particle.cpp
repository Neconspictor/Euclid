#include <nex/particle/Particle.hpp>
#include <glm/glm.hpp>
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
#include <nex/texture/Texture.hpp>

#include <glm/gtx/compatibility.hpp>
#include <nex/math/Math.hpp>

#undef max
#undef min

nex::Particle::Particle(const glm::vec3& pos, 
	const glm::vec3& vel, 
	const glm::vec3& dampedVel,
	float rotation, 
	float scale, 
	float lifeTime, 
	float gravityInfluence) :

	mPosition(pos), 
	mTargetPosition(pos),
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
	mTargetPosition = mPosition;
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

bool nex::Particle::update(const glm::vec3& velocity, float frameTime)
{	
	mElapsedTime += frameTime;

	mVelocity.y += GRAVITY * mGravityInfluence* frameTime;

	//mPosition += mVelocity * frameTime;// +velocity * std::max<float>((1.0f - mElapsedTime / mLifeTime), 0.1f);
	mPosition += mVelocity * frameTime;// +velocity * std::max<float>((1.0f - mElapsedTime / mLifeTime), 0.1f);
	mTargetPosition += mVelocity * frameTime + velocity;
	auto diff = mTargetPosition - mPosition;

	float factor = length(velocity);// *mElapsedTime / mLifeTime;
	//mPosition += diff;
	//mPosition = glm::mix(mPosition, mTargetPosition, frameTime * 2.0f);

	static const float DAMPING = 1.0f;
	const float alpha = std::clamp<float>(frameTime * DAMPING + (1.0f - mElapsedTime / mLifeTime), 0.0f, 1.0f);
	//auto position = glm::mix(mPosition, mTargetPosition, 1.0f - mElapsedTime / mLifeTime);
	mPosition = glm::mix(mPosition, mTargetPosition, alpha);
	//lerp between position and target position

	mIsAlive = mElapsedTime < mLifeTime;
	return mIsAlive;
}

void nex::Particle::updateWorldTrafo(const glm::mat4& invViewWithoutPosition)
{
	auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(mScale));
	auto rotation = glm::rotate(glm::mat4(1.0f), mRotation, glm::vec3(0, 0, 1));
	mWorldTrafo = glm::mat4(1.0f);
	mWorldTrafo[3] = glm::vec4(mPosition, 1.0f);
	mWorldTrafo = mWorldTrafo * invViewWithoutPosition * rotation * scale;
}


nex::ParticleShader::Material::Material(nex::Shader* shader) : nex::Material(shader) 
{

}

nex::ParticleShader::ParticleShader() : 
	Shader(nex::ShaderProgram::create("particle/particle_vs.glsl", "particle/particle_fs.glsl"))
{
	mViewProj = { mProgram->getUniformLocation("viewProj"), UniformType::MAT4 };
	mInverseView3x3 = { mProgram->getUniformLocation("invView3x3"), UniformType::MAT3 };
	mModel = { mProgram->getUniformLocation("model"), UniformType::MAT4 };

	mTexture = mProgram->createTextureUniform("tex", UniformType::TEXTURE2D, 0);
	mColor = { mProgram->getUniformLocation("baseColor"), UniformType::VEC4 };
	mTileCount = { mProgram->getUniformLocation("tileCount"), UniformType::UVEC2 };
	mLifeTimePercentage = { mProgram->getUniformLocation("lifeTimePercentage"), UniformType::FLOAT };
}

void nex::ParticleShader::setLifeTimePercentage(float percentage)
{
	mProgram->setFloat(mLifeTimePercentage.location, percentage);
}

void nex::ParticleShader::updateConstants(const Constants& constants) {
	auto viewProj = constants.camera->getProjectionMatrix() * constants.camera->getView();
	mProgram->setMat4(mViewProj.location, viewProj);
	const auto& invView = constants.camera->getViewInv();
	mProgram->setMat3(mInverseView3x3.location, invView);
}

void nex::ParticleShader::updateInstance(const glm::mat4& modelMatrix, const glm::mat4& prevModelMatrix, const void* data) {
	mProgram->setMat4(mModel.location, modelMatrix);

	if (data != nullptr) {
		const auto* particle = (const Particle*)data;
		auto percentage = particle->getElapsedTime() / particle->getLifeTime();
		setLifeTimePercentage(std::min<float>(percentage, 1.0f));
	}
}

void nex::ParticleShader::updateMaterial(const nex::Material& m) {
	const ParticleShader::Material* material = nullptr;
	try {
		material = &dynamic_cast<const ParticleShader::Material&>(m);
	}
	catch (const std::bad_cast & e) {
		throw_with_trace(e);
	}

	mProgram->setVec4(mColor.location, material->color);

	if (material->texture) {
		mProgram->setTexture(material->texture, Sampler::getDefaultImage(), mTexture.bindingSlot);
		mProgram->setUVec2(mTileCount.location, material->texture->getTileCount());
	}

	auto* instanceBuffer = material->instanceBuffer;
	if (instanceBuffer) {
		bindParticlesBuffer(instanceBuffer);
	}
	
}

void nex::ParticleShader::bindParticlesBuffer(ShaderStorageBuffer* buffer)
{
	buffer->bindToTarget(0);
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
	size_t activeParticleCount,
	const nex::AABB* boundingBox,
	RenderCommandQueue& commandQueue,
	bool doCulling)
{
	auto command = mPrototype;

	if (!activeParticleCount) return;

	command.boundingBox = boundingBox;
	command.instanceCount = activeParticleCount;
	commandQueue.push(command, doCulling);
}

nex::RenderState nex::ParticleRenderer::createParticleRenderState()
{
	RenderState state;
	state.doBlend = true;
	state.blendDesc = { BlendFunc::SOURCE_ALPHA, BlendFunc::ONE, BlendOperation::ADD };
	state.doDepthWrite = false;
	state.doShadowCast = false;
	state.doShadowReceive = false;
	state.doCullFaces = false;

	return state;
}

void nex::ParticleRenderer::createParticleMaterial(Material* material)
{
	material->getRenderState() = createParticleRenderState();
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

	for (auto& pair : command.batch->getEntries()) {
		Drawer::draw(currentShader, pair.first, pair.second, overwriteState, command.instanceCount);
	}
}

nex::ParticleRenderer::~ParticleRenderer() = default;


nex::ParticleManager::ParticleManager(size_t maxParticles) : mLastActive(-1), mParticles(maxParticles)
{
}

void nex::ParticleManager::create(const glm::vec3& pos, 
	const glm::vec3& vel, 
	const glm::vec3& dampedVel,
	float rotation, 
	float scale, 
	float lifeTime, 
	float gravityInfluence)
{
	const auto nextSize = mLastActive + 1;
	if (nextSize >= mParticles.size()) return;

	++mLastActive;
	mParticles[mLastActive] = Particle(pos, vel, dampedVel, rotation, scale, lifeTime, gravityInfluence);
}

void nex::ParticleManager::frameUpdate(const glm::vec3& velocity, float frameTime)
{
	// Update all particles and retrieve the highest index of active particles.
	for (int i = mLastActive; i >= 0; --i) {
		auto isAlive = mParticles[i].update(velocity, frameTime);
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

	mBoundingBox = AABB();

	for (int i = 0; i <= mLastActive; ++i) {
		// Update Bounding box
		mBoundingBox.min = nex::minVec(mParticles[i].getPosition(), mBoundingBox.min);
		mBoundingBox.max = nex::maxVec(mParticles[i].getPosition(), mBoundingBox.max);
	}
}

void nex::ParticleManager::updateParticleTrafos(const glm::mat4& invViewWithoutPosition)
{
	for (int i = 0; i <= mLastActive; ++i) {
		mParticles[i].updateWorldTrafo(invViewWithoutPosition);
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

const nex::Particle* nex::ParticleManager::getParticles() const
{
	return mParticles.data();
}

size_t nex::ParticleManager::getActiveParticleCount() const
{
	return static_cast<size_t>(mLastActive + 1);
}

size_t nex::ParticleManager::getBufferSize() const
{
	return mParticles.size();
}

const nex::AABB& nex::ParticleManager::getBoundingBox() const
{
	return mBoundingBox;
}

void nex::ParticleManager::sortActiveParticles(const glm::vec3& cameraPosition)
{
	if (mLastActive < 0) return;

	std::sort(mParticles.begin(), mParticles.begin() + mLastActive+1, [&cameraPosition](const Particle& a, const Particle& b) {
	
		auto compareA = glm::distance2(a.getPosition(), cameraPosition);
		auto compareB = glm::distance2(b.getPosition(), cameraPosition);
		return compareA > compareB;
	});
}

nex::VarianceParticleSystem::VarianceParticleSystem(
	float averageLifeTime, 
	float averageScale, 
	float averageSpeed, 
	const AABB& boundingBox, 
	float gravityInfluence, 
	std::unique_ptr<ParticleShader::Material> material,
	size_t maxParticles, 
	const glm::vec3& position, 
	float pps, 
	float rotation, 
	bool randomizeRotation,
	bool sortParticles) :
	Vob(nullptr),
	FrameUpdateable(),
	mAverageLifeTime(averageLifeTime),
	mAverageScale(averageScale),
	mAverageSpeed(averageSpeed),
	mDirection(glm::vec3(0.0f)),
	mGravityInfluence(gravityInfluence),
	mManager(maxParticles),
	mMaterial(std::move(material)),
	mPartialParticles(0.0f),
	mPps(pps),
	mRotation(rotation),
	mRandomizeRotation(randomizeRotation),
	mSortParticles(sortParticles),
	mRenderer(mMaterial.get())
{
	mUseCone = length(mDirection) != 0.0f;
	mInstanceBuffer = std::make_unique<ShaderStorageBuffer>(0, maxParticles * sizeof(ParticleShader::ParticleData), 
		nullptr, GpuBuffer::UsageHint::DYNAMIC_DRAW);

	mShaderParticles.resize(maxParticles);
	mMaterial->instanceBuffer = mInstanceBuffer.get();
	setPosition(position);
	
}

void nex::VarianceParticleSystem::collectRenderCommands(RenderCommandQueue& queue, bool doCulling, ShaderStorageBuffer* boneTrafoBuffer)
{
	mRenderer.createRenderCommands(mManager.getActiveParticleCount(),
		&mManager.getBoundingBox(),
		queue,
		doCulling);
}

void nex::VarianceParticleSystem::frameUpdate(const Constants& constants)
{
	const auto& frameTime = constants.frameTime;
	auto particlesToCreate = mPps * frameTime;
	double count1;
	mPartialParticles += std::modf(particlesToCreate, &count1);

	double count2;
	mPartialParticles = std::modf(mPartialParticles, &count2);

	size_t count = static_cast<size_t>(count1 + count2);

	auto psVelocity = mPosition - mOldPosition;
	mOldPosition = mPosition;

	mManager.frameUpdate(psVelocity, frameTime);
	recalculateLocalBoundingBox();

	emit(mPosition, psVelocity, count);

	auto invViewWithoutPosition = glm::mat4(transpose(glm::mat3(constants.camera->getView())));
	invViewWithoutPosition[3][3] = 1.0f;

	if (mSortParticles) mManager.sortActiveParticles(constants.camera->getPosition());
	mManager.updateParticleTrafos(invViewWithoutPosition);


	if (mManager.getActiveParticleCount() > 0) {

		const auto* particles = mManager.getParticles();

		for (int i = 0; i < mManager.getActiveParticleCount(); ++i) {
			auto& data = mShaderParticles[i];
			const auto& particle = particles[i];

			data.worldTrafo = particle.getWorldTrafo();
			data.lifeTimePercentage = particle.getElapsedTime() / particle.getLifeTime();
		}

		mInstanceBuffer->update(mManager.getActiveParticleCount() * sizeof(ParticleShader::ParticleData), mShaderParticles.data(), 0);
	}	
}

void nex::VarianceParticleSystem::setDirection(const glm::vec3& direction, float directionDeviation)
{
	mDirectionDeviation = directionDeviation;
	mDirection = normalize(direction);
	mUseCone = length(direction) != 0.0f;
}

void nex::VarianceParticleSystem::setLifeVariance(float variance)
{
	mLifeVariance = variance * mAverageLifeTime;
}

void nex::VarianceParticleSystem::setScaleVariance(float variance)
{
	mScaleVariance = variance * mAverageScale;
}

void nex::VarianceParticleSystem::setSpeedVariance(float variance)
{
	mSpeedVariance = variance * mAverageSpeed;
}

void nex::VarianceParticleSystem::emit(const glm::vec3& center, const glm::vec3& psVelocity, size_t count)
{

	for (size_t i = 0; i < count; ++i) {
		glm::vec3 velocity;
		if (mUseCone) {
			velocity = generateRandomUnitVectorWithinCone(mDirection, mDirectionDeviation);
		}
		else {
			velocity = generateRandomUnitVector();
		}
		velocity = normalize(velocity);
		velocity *= generateValue(mAverageSpeed, mSpeedVariance);
		float scale = generateValue(mAverageScale, mScaleVariance);
		float lifeTime = generateValue(mAverageLifeTime, mLifeVariance);
		float rotation = mRotation + generateRotation();

		mManager.create(center, velocity, psVelocity, rotation, scale, lifeTime, mGravityInfluence);
	}
}

glm::vec3 nex::VarianceParticleSystem::generateRandomUnitVector()
{
	glm::vec3 vec;
	float theta = (float)(Random::nextFloat() * TWO_PI);
	vec.z = (Random::nextFloat() * 2.0f) - 1.0f;
	float rootOneMinusZSquared = std::sqrtf(1.0f - vec.z * vec.z);
	vec.x = rootOneMinusZSquared * std::cosf(theta);
	vec.y = rootOneMinusZSquared * std::sinf(theta);
	return vec;
}

glm::vec3 nex::VarianceParticleSystem::generateRandomUnitVectorWithinCone(const glm::vec3& dir, float angle)
{
	float cosAngle = std::cosf(angle);
	float theta = Random::nextFloat() * TWO_PI;
	float z = cosAngle + Random::nextFloat() * (1.0f - cosAngle);
	float rootOneMinusZSquared = std::sqrtf(1.0f - z * z);
	float x = rootOneMinusZSquared * std::cosf(theta);
	float y = rootOneMinusZSquared * std::sinf(theta);

	glm::vec4 direction (x, y, z, 1); // TODO: w comp should be 0?

	//if (dir.x != 0.0f || dir.y != 0.0f || (dir.z != 1.0f && dir.z != -1.0f)) {
		glm::vec3 rotateAxis = normalize(glm::cross(dir, glm::vec3(0.0f, 0.0f, 1.0f)));
		float rotateAngle = std::acosf(glm::dot(dir, glm::vec3(0.0f, 0.0f, 1.0f)));
		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), -rotateAngle, rotateAxis);
		direction = rotationMatrix * direction;
	//}
	//else if (dir.z == -1.0f) {
	//	direction.z *= -1.0f;
	//}

	return direction;
}

float nex::VarianceParticleSystem::generateRotation() const
{
	if (mRandomizeRotation) {
		return Random::nextFloat() * TWO_PI;
	}
	else {
		return 0.0f;
	}
}

float nex::VarianceParticleSystem::generateValue(float average, float variance) {
	float offset = (nex::Random::nextFloat() - 0.5f) * 2.0f * variance;
	return average + offset;
}

void nex::VarianceParticleSystem::recalculateLocalBoundingBox()
{
	mBoundingBoxLocal = mManager.getBoundingBox();
	mBoundingBoxLocal.min -= mPosition;
	mBoundingBoxLocal.max -= mPosition;
}

void nex::VarianceParticleSystem::recalculateBoundingBoxWorld()
{
	//TODO: rotation and scale don't affect the particles?
	//mBoundingBoxWorld = mBoundingBoxLocal;
	Vob::recalculateBoundingBoxWorld();
	
}
