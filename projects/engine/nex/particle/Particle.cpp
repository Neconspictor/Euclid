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

	glm::vec3 change = mVelocity * frameTime + mPosition;

	mIsAlive = mElapsedTime < mLifeTime;

	auto mWorldTrafo = glm::translate(glm::mat4(), mPosition);
	mWorldTrafo[0][0] = view[0][0];
	mWorldTrafo[0][1] = view[1][0];
	mWorldTrafo[0][2] = view[2][0];
	mWorldTrafo[1][0] = view[0][1];
	mWorldTrafo[1][1] = view[1][1];
	mWorldTrafo[2][0] = view[0][2];
	mWorldTrafo[2][1] = view[1][2];
	mWorldTrafo[2][2] = view[2][2];



	mWorldTrafo = glm::rotate(mWorldTrafo, mRotation, glm::vec3(0,0,1));
	mWorldTrafo = glm::scale(mWorldTrafo, glm::vec3(mScale));

	return mIsAlive;
}

class nex::ParticleRenderer::ParticleShader : public nex::TransformShader {
public:
	ParticleShader() : TransformShader(nex::ShaderProgram::create("particle/particle_vs.glsl", "particle/particle_fs.glsl")) 
	{

	}
};

nex::ParticleRenderer::ParticleRenderer()
{

	mShader = std::make_unique<ParticleShader>();


	static const float planeVertices[] = {
		// position 2 floats
		-0.5f, 0.5f,
		-0.5f, -0.5f,
		0.5f, 0.5f,
		0.5f, -0.5
	};

	auto vertexBuffer = std::make_unique<VertexBuffer>(sizeof(planeVertices), planeVertices);
	VertexLayout layout;

	layout.push<float>(2, vertexBuffer.get(), false, false, true);

	auto mesh = std::make_unique<Mesh>();
	mesh->addVertexDataBuffer(std::move(vertexBuffer));
	mesh->setLayout(std::move(layout));
	mesh->setTopology(Topology::TRIANGLE_STRIP);
	mesh->setUseIndexBuffer(false);
	mesh->setVertexCount(sizeof(planeVertices) / (2 * sizeof(float)));
	
	auto material = std::make_unique<Material>(mShader.get());
	
	mParticleMG = std::make_unique<MeshGroup>();
	
	mParticleMG->addMapping(mesh.get(), material.get());
	mParticleMG->add(std::move(mesh));
	mParticleMG->addMaterial(std::move(material));


	nex::ResourceLoader::get()->enqueue([groupPtr = mParticleMG.get()](nex::RenderEngine::CommandQueue* commandQueue)-> nex::Resource* {
		commandQueue->push([=]() {
			groupPtr->finalize();
		});

		return nullptr;
	});

	mParticleMG->calcBatches();
	auto* batches = mParticleMG->getBatches();

	for (auto& batch : *batches) {
		nex::RenderCommand command;
		command.isBoneAnimated = false;
		command.batch = &batch;
		mPrototypes.push_back(command);
	}
	
}

void nex::ParticleRenderer::createRenderCommands(const std::vector<Particle>& particles, RenderCommandQueue& commandQueue)
{
	for (const auto& particle : particles) {
		for (auto prototype : mPrototypes) {
			prototype.worldTrafo = &particle.getWorldTrafo();
			prototype.prevWorldTrafo = prototype.worldTrafo;
			commandQueue.push(prototype);
		}
	}
}

nex::ParticleRenderer::~ParticleRenderer() = default;