#include <nex/pbr/Cluster.hpp>
#include <nex/mesh/UtilityMeshes.hpp>
#include <nex/mesh/StaticMesh.hpp>
#include <nex/material/Material.hpp>
#include <nex/shader/Technique.hpp>
#include <nex/shader/SimpleColorPass.hpp>
#include <nex/Scene.hpp>
#include <nex/gui/Util.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/math/Math.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/buffer/ShaderBuffer.hpp>
#include <nex/Window.hpp>
#include <nex/EffectLibrary.hpp>
#include <nex/renderer/RenderBackend.hpp>


class nex::ClusterGenerator::GenerateClusterPass : public nex::ComputePass {
public:
	GenerateClusterPass() :
		ComputePass(Shader::createComputeShader("cluster/clusters_cs.glsl")),
		mConstants(std::make_unique<ShaderStorageBuffer>(getConstantsBufferBingingPoint(), sizeof(Constants), nullptr, GpuBuffer::UsageHint::STREAM_DRAW))
	{

	}

	void setClusters(ShaderStorageBuffer* buffer) {
		buffer->bindToTarget(0);
	}

	ShaderStorageBuffer* getConstantsBuffer() {
		return mConstants.get();
	}

	static unsigned getConstantsBufferBingingPoint() {
		return 1;
	}

private:
	std::unique_ptr<ShaderStorageBuffer> mConstants;
};

nex::ClusterGenerator::ClusterGenerator() : 
	mGenerateClusterPass(std::make_unique<GenerateClusterPass>())
{
}

nex::ClusterGenerator::~ClusterGenerator() = default;

void nex::ClusterGenerator::generateClusters(ShaderStorageBuffer* output, const glm::uvec3& clusterSize, const Constants& constants)
{

	const auto size = clusterSize.x * clusterSize.y * clusterSize.z * sizeof(ClusterAABB);

	output->resize(size, nullptr, nex::GpuBuffer::UsageHint::STREAM_DRAW);
	
	mGenerateClusterPass->bind();
	mGenerateClusterPass->setClusters(output);
	mGenerateClusterPass->getConstantsBuffer()->update(sizeof(Constants), &constants);
	mGenerateClusterPass->getConstantsBuffer()->bindToTarget();

	mGenerateClusterPass->dispatch(clusterSize.x, clusterSize.y, clusterSize.z);
}

class nex::EnvLightCuller::CullPass : public nex::ComputePass {
public:

	struct Constants {
		glm::mat4 viewMatrix;
	};

	CullPass() :
		ComputePass(Shader::createComputeShader("cluster/cull_environment_lights_cs.glsl")),
		mConstants(std::make_unique<UniformBuffer>(0, sizeof(Constants), nullptr, GpuBuffer::UsageHint::STREAM_DRAW))
	{

	}

	void setClusters(ShaderStorageBuffer* buffer) {
		buffer->bindToTarget(0);
	}

	void setConstants(const Constants& constants) {
		mConstants->update(sizeof(Constants), &constants);
		mConstants->bindToTarget();
	}

	void setEnvironmentLights(ShaderStorageBuffer* buffer) {
		buffer->bindToTarget(1);
	}

	void setGlobalLightIndexList(ShaderStorageBuffer* buffer) {
		buffer->bindToTarget(2);
	}

	void setLightGrids(ShaderStorageBuffer* buffer) {
		buffer->bindToTarget(3);
	}

	void setGlobalIndexCount(ShaderStorageBuffer* buffer) {
		buffer->bindToTarget(4);
	}

private:
	std::unique_ptr<UniformBuffer> mConstants;
};


nex::EnvLightCuller::EnvLightCuller() :
	mGlobalLightIndexCountBuffer(std::make_unique<ShaderStorageBuffer>(4, sizeof(GlobalLightIndexCount), nullptr, GpuBuffer::UsageHint::DYNAMIC_DRAW)),
	mGlobalLightIndexListBuffer(std::make_unique<ShaderStorageBuffer>(2, 0, nullptr, GpuBuffer::UsageHint::STREAM_COPY)),
	mLightGridsBuffer(std::make_unique<ShaderStorageBuffer>(3, 0, nullptr, GpuBuffer::UsageHint::STREAM_COPY)),
	mCullPass(std::make_unique<CullPass>())
{
}

nex::EnvLightCuller::~EnvLightCuller() = default;

nex::ShaderStorageBuffer* nex::EnvLightCuller::getGlobalLightIndexCount()
{
	return mGlobalLightIndexCountBuffer.get();
}

nex::ShaderStorageBuffer* nex::EnvLightCuller::getGlobalLightIndexList()
{
	return mGlobalLightIndexListBuffer.get();
}

nex::ShaderStorageBuffer* nex::EnvLightCuller::getLightGrids()
{
	return mLightGridsBuffer.get();
}

void nex::EnvLightCuller::cullLights(const glm::mat4& viewMatrix, ShaderStorageBuffer* clusters, const glm::vec3& clusterSize, ShaderStorageBuffer* envLights)
{
	mCullPass->bind();


	const auto flattenedSize = clusterSize.x * clusterSize.y * clusterSize.z;

	mGlobalLightIndexCountBuffer->resize(flattenedSize * MAX_VISIBLES_LIGHTS_PER_CLUSTER * sizeof(GlobalLightIndexCount), nullptr, GpuBuffer::UsageHint::STREAM_COPY);
	mLightGridsBuffer->resize(flattenedSize * sizeof(ClusterLightGrid), nullptr, GpuBuffer::UsageHint::STREAM_COPY);


	mCullPass->setClusters(clusters);
	mCullPass->setConstants((const CullPass::Constants&) viewMatrix);
	mCullPass->setEnvironmentLights(envLights);
	mCullPass->setGlobalIndexCount(mGlobalLightIndexCountBuffer.get());
	mCullPass->setGlobalLightIndexList(mGlobalLightIndexListBuffer.get());
	mCullPass->setLightGrids(mLightGridsBuffer.get());
	

	// TODO : Generify dispatch size!
	mCullPass->dispatch(1,1, clusterSize.z / 4 );
}


class nex::ProbeCluster::CollectClustersPass : public nex::ComputePass {
public:

	CollectClustersPass() :
		ComputePass(Shader::createComputeShader("cluster/collect_clusters_cs.glsl")),
		mBuffer(std::make_unique<ShaderStorageBuffer>(0, 0, nullptr, GpuBuffer::UsageHint::STREAM_DRAW))
	{
		auto state = mDepthSampler.getState();
		state.minFilter = state.magFilter = TextureFilter::NearestNeighbor;
		mDepthSampler.setState(state);
	}

	struct ActiveClusters {
		glm::uvec4 numClusters; // cluster dimension in x,y and z axis; w component is unused
		glm::vec4 constantsAB; // x: log(zFarDistance / zNearDistance), y: log(zNearDistance) * numClusters.z / log(zFarDistance/zNearDistance)

		// The remaining of the buffer is an unsigned int array of 
		// clusterSize.xSize * clusterSize.ySize * clusterSize.zSize elements.
	};

	void resizeBuffer(const glm::uvec3& clusterSize) {
		const auto flattenedSize = clusterSize.x * clusterSize.y * clusterSize.z;
		mBuffer->resize(sizeof(ActiveClusters) + flattenedSize * sizeof(unsigned), nullptr, GpuBuffer::UsageHint::STREAM_DRAW);
	}

	void setDepthTexture(Texture* texture) {
		mShader->setTexture(texture, &mDepthSampler, 0);
	}

	ShaderStorageBuffer* getBuffer() {
		return mBuffer.get();
	}


private:

	std::unique_ptr<ShaderStorageBuffer> mBuffer;
	nex::Sampler mDepthSampler;
};


class nex::ProbeCluster::CleanClusterListPass : public nex::ComputePass {
public:

	struct OutputHeader {
		glm::uint globalActiveClusterCount;
	};

	CleanClusterListPass() :
		ComputePass(Shader::createComputeShader("cluster/clean_cluster_list_cs.glsl")),
		mOutput(std::make_unique<ShaderStorageBuffer>(1, 0, nullptr, GpuBuffer::UsageHint::STREAM_DRAW))
	{

	}

	unsigned getInputBindingPoint() {
		return 0;
	}

	ShaderStorageBuffer* getOutput() {
		return mOutput.get();
	}

private:
	std::unique_ptr<ShaderStorageBuffer> mOutput;
};


class nex::ProbeCluster::CullLightsPass : public nex::ComputePass {
public:

	struct Constants {
		glm::mat4 viewMatrix;
	};

	CullLightsPass() :
		ComputePass(Shader::createComputeShader("cluster/cull_point_lights_cs.glsl")),
		mConstants(std::make_unique<UniformBuffer>(0, sizeof(Constants), nullptr, GpuBuffer::UsageHint::STREAM_DRAW))
	{

	}

	UniformBuffer* getConstants() {
		return mConstants.get();
	}

private:
	std::unique_ptr<UniformBuffer> mConstants;
};


nex::ProbeCluster::ProbeCluster(Scene* scene) : mScene(scene), 
mMaterial(RenderBackend::get()->getEffectLibrary()->createSimpleColorMaterial()),
mClusterAABBBuffer(std::make_unique<ShaderStorageBuffer>(0, 
	0, // Note: we dynamically resize the buffer 
	nullptr, 
	GpuBuffer::UsageHint::DYNAMIC_COPY)),
mCollectClustersPass(std::make_unique<CollectClustersPass>()),
mCleanClusterListPass(std::make_unique<CleanClusterListPass>()),
mCullLightsPass(std::make_unique<CullLightsPass>())
{
	mMaterial->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 0.3f));

	auto& state = mMaterial->getRenderState();
	state.fillMode = FillMode::FILL;
	state.doCullFaces = false;
	state.isTool = false;
	state.doShadowCast = false;
	state.doShadowReceive = false;
}

nex::ProbeCluster::~ProbeCluster() = default;

nex::PerspectiveCamera& nex::ProbeCluster::getCamera()
{
	return mCamera;
}

void nex::ProbeCluster::generate(const Frustum& frustum)
{
	auto mesh = std::make_unique<FrustumMesh>(frustum);

	auto container = std::make_unique<StaticMeshContainer>();

	container->addMapping(mesh.get(), mMaterial.get());
	container->add(std::move(mesh));
	container->finalize();

	mScene->acquireLock();
	mScene->addVobUnsafe(std::make_unique<MeshOwningVob>(std::move(container)), true);
}

void nex::ProbeCluster::generateClusterElement(const ClusterElement& elem)
{
	mCamera.update();
	auto frustumView = mCamera.calcClusterElementViewSpace(elem.xOffset, elem.yOffset, elem.zOffset, elem.depth, elem.width, elem.height);
	const auto viewInv = inverse(mCamera.getView());
	const auto frustumWorld = frustumView * viewInv;
	generate(frustumWorld);

}

void nex::ProbeCluster::generateCluster(const glm::uvec3& clusterSize, unsigned width, unsigned height)
{
	//generateClusterCpuTest(clusterSize); return;
	generateClusterGpu(clusterSize, width, height); return;
	mCamera.update();
	ClusterElement elem;
	auto container = std::make_unique<StaticMeshContainer>();

	elem.width = 1.0f / (float) clusterSize.x;
	elem.height = 1.0f / (float) clusterSize.y;
	elem.depth = 1.0f / (float) clusterSize.z;
	const auto viewInv = inverse(mCamera.getView());

	const auto middleDistance = (mCamera.getFarDistance() + mCamera.getNearDistance()) / 2.0f;
	const auto lookVecView = glm::vec3(0,0, Camera::getViewSpaceZfromDistance(1.0f));
	const auto middleTrans = glm::translate(glm::mat4(), -middleDistance * lookVecView);


	for (size_t x = 0; x < clusterSize.x; ++x ) {
		for (size_t y = 0; y < clusterSize.y; ++y) {
			for (size_t z = 0; z < clusterSize.z; ++z) {

				elem.xOffset = x * elem.width;
				elem.yOffset = y * elem.height;
				elem.zOffset = z * elem.depth;

				auto frustumView = mCamera.calcClusterElementViewSpace(elem.xOffset, elem.yOffset, elem.zOffset, elem.depth, elem.width, elem.height);
				
				//const auto frustumWorld = frustumView * viewInv;
				//auto mesh = std::make_unique<FrustumMesh>(frustumWorld);
				frustumView = middleTrans * frustumView;
				auto mesh = std::make_unique<MeshAABB>(frustumView.calcAABB(), Topology::LINES);
				//auto mesh = std::make_unique<FrustumMesh>(frustumView);

				container->addMapping(mesh.get(), mMaterial.get());
				container->add(std::move(mesh));
			}
		}
	}

	container->finalize();
	container->merge();
	mScene->acquireLock();

	auto vob = std::make_unique<MeshOwningVob>(std::move(container));
	vob->setTrafo(viewInv);
	const auto& look = mCamera.getLook();
	const auto middlePoint = (mCamera.getFarDistance() + mCamera.getNearDistance()) / 2.0f * look;
	vob->setPosition(vob->getPosition() + middlePoint);
	vob->updateTrafo(true);

	mScene->addVobUnsafe(std::move(vob), true);

}

void nex::ProbeCluster::generateClusterCpuTest(const glm::uvec3& clusterSize)
{
	mCamera.update();
	ClusterElement elem;
	auto container = std::make_unique<StaticMeshContainer>();

	elem.width = 1.0f / (float)clusterSize.x;
	elem.height = 1.0f / (float)clusterSize.y;
	elem.depth = 1.0f / (float)clusterSize.z;
	const auto viewInv = inverse(mCamera.getView());

	const auto middleDistance = (mCamera.getFarDistance() + mCamera.getNearDistance()) / 2.0f;
	const auto lookVecView = glm::vec3(0, 0, Camera::getViewSpaceZfromDistance(1.0f));
	const auto middleTrans = glm::translate(glm::mat4(), -middleDistance * lookVecView);

	const glm::vec2 zNearFar (Camera::getViewSpaceZfromDistance(mCamera.getNearDistance()), 
		Camera::getViewSpaceZfromDistance(mCamera.getFarDistance()));
	const glm::uvec4 tileSizes(1920 / clusterSize.x);
	const auto invProj = inverse(mCamera.getProjectionMatrix());
	const glm::vec2 screenDimension(1920, 1080);


	for (size_t x = 0; x < clusterSize.x; ++x) {
		for (size_t y = 0; y < clusterSize.y; ++y) {
			for (size_t z = 0; z < clusterSize.z; ++z) {

				elem.xOffset = x * elem.width;
				elem.yOffset = y * elem.height;
				elem.zOffset = z * elem.depth;

				//auto frustumView = mCamera.calcClusterElementViewSpace(elem.xOffset, elem.yOffset, elem.zOffset, elem.depth, elem.width, elem.height);

				//const auto frustumWorld = frustumView * viewInv;
				//auto mesh = std::make_unique<FrustumMesh>(frustumWorld);
				//frustumView = middleTrans * frustumView;
				auto box = main(glm::vec3(x,y,z), clusterSize, zNearFar,tileSizes, invProj,
					screenDimension);
				box.min = glm::vec3(middleTrans * glm::vec4(box.min, 1.0f));
				box.max = glm::vec3(middleTrans * glm::vec4(box.max, 1.0f));

				auto mesh = std::make_unique<MeshAABB>(box, Topology::LINES);


				container->addMapping(mesh.get(), mMaterial.get());
				container->add(std::move(mesh));
			}
		}
	}

	container->finalize();
	container->merge();
	mScene->acquireLock();

	auto vob = std::make_unique<MeshOwningVob>(std::move(container));
	vob->setTrafo(viewInv);
	const auto& look = mCamera.getLook();
	const auto middlePoint = (mCamera.getFarDistance() + mCamera.getNearDistance()) / 2.0f * look;
	vob->setPosition(vob->getPosition() + middlePoint);
	vob->updateTrafo(true);

	mScene->addVobUnsafe(std::move(vob), true);
}

void nex::ProbeCluster::generateClusterGpu(const glm::uvec3& clusterSize, unsigned width, unsigned height)
{
	mCamera.update();
	const auto viewInv = inverse(mCamera.getView());
	const auto middleDistance = (mCamera.getFarDistance() + mCamera.getNearDistance()) / 2.0f;
	const auto lookVecView = glm::vec3(0, 0, Camera::getViewSpaceZfromDistance(1.0f));
	const auto middleTrans = glm::translate(glm::mat4(), -middleDistance * lookVecView);
	const auto flattenedClusterSize = clusterSize.x * clusterSize.y * clusterSize.z;

	auto container = std::make_unique<StaticMeshContainer>();
	
	// generate clusters
	ClusterGenerator::Constants constants;
	constants.invProj = inverse(mCamera.getProjectionMatrix());
	constants.invView = viewInv;
	constants.zNearFar = glm::vec4(Camera::getViewSpaceZfromDistance(mCamera.getNearDistance()),
		Camera::getViewSpaceZfromDistance(mCamera.getFarDistance()), 0, 0);
	mClusterGenerator.generateClusters(mClusterAABBBuffer.get(), clusterSize, constants);


	// Readback the generated clusters
	auto* clusters = (nex::ClusterAABB*)mClusterAABBBuffer->map(GpuBuffer::Access::READ_ONLY);
		for (unsigned i = 0; i < flattenedClusterSize; ++i) {
			auto& cluster = clusters[i];

			AABB box = {glm::vec3(cluster.minView), glm::vec3(cluster.maxView)};

			//box.min = glm::vec3(middleTrans * glm::vec4(box.min, 1.0f));
			//box.max = glm::vec3(middleTrans * glm::vec4(box.max, 1.0f));

			auto mesh = std::make_unique<MeshAABB>(box, Topology::LINES);
			container->addMapping(mesh.get(), mMaterial.get());
			container->add(std::move(mesh));
		}
	mClusterAABBBuffer->unmap();

	container->finalize();
	container->merge();
	mScene->acquireLock();

	auto vob = std::make_unique<MeshOwningVob>(std::move(container));
	vob->setTrafo(viewInv);
	const auto& look = mCamera.getLook();
	const auto middlePoint = (mCamera.getFarDistance() + mCamera.getNearDistance()) / 2.0f * look;
	//vob->setPosition(vob->getPosition() + middlePoint);
	vob->updateTrafo(true);

	mScene->addVobUnsafe(std::move(vob), true);


	collectActiveClusterGpuTest(clusterSize, mCamera.getNearDistance(), mCamera.getFarDistance(), width, height);
	cleanActiveClusterListGpuTest(clusterSize, mCollectClustersPass->getBuffer());
}

void nex::ProbeCluster::collectActiveClusterGpuTest(const glm::uvec3& clusterSize,
	float zNearDistance, 
	float zFarDistance,
	unsigned width,
	unsigned height)
{
	mCollectClustersPass->bind();
	
	mCollectClustersPass->resizeBuffer(clusterSize);
	auto* buffer = mCollectClustersPass->getBuffer();

	void* data = buffer->map(GpuBuffer::Access::READ_WRITE);
		auto* activeClusters = (CollectClustersPass::ActiveClusters*)data;
		activeClusters->numClusters = glm::uvec4(clusterSize.x, clusterSize.y, clusterSize.z, 0);

		auto logzFar_zNear = logf(zFarDistance / zNearDistance);
		activeClusters->constantsAB = glm::vec4(logzFar_zNear, logf(zNearDistance) * clusterSize.z / logzFar_zNear, 0.0f, 0.0f);
	
		auto* flagArray = (unsigned*)((char*)data + sizeof(CollectClustersPass::ActiveClusters));
		const auto flattenedSize = clusterSize.x * clusterSize.y * clusterSize.z;
		memset(flagArray, 0, flattenedSize * sizeof(unsigned));
	buffer->unmap();

	buffer->bindToTarget();
	mCollectClustersPass->dispatch(width, height, 1);

	data = buffer->map(GpuBuffer::Access::READ_WRITE);
		activeClusters = (CollectClustersPass::ActiveClusters*)data;
		flagArray = (unsigned*)((char*)data + sizeof(CollectClustersPass::ActiveClusters));
	buffer->unmap();
}

void nex::ProbeCluster::cleanActiveClusterListGpuTest(const glm::uvec3& clusterSize, ShaderStorageBuffer* activeClusters)
{
	mCleanClusterListPass->bind();


	auto* output = mCleanClusterListPass->getOutput();
	unsigned defaultGlobalActiveClusterCount = 0;
	const auto flattenedSize = clusterSize.x * clusterSize.y * clusterSize.z;
	output->resize(sizeof(unsigned) * (flattenedSize + 1), nullptr, GpuBuffer::UsageHint::STREAM_DRAW);
	output->update(sizeof(unsigned), &defaultGlobalActiveClusterCount);

	activeClusters->bindToTarget(mCleanClusterListPass->getInputBindingPoint());
	output->bindToTarget();

	mCleanClusterListPass->dispatch(clusterSize.x, clusterSize.y, clusterSize.z);

	void* data = output->map(GpuBuffer::Access::READ_ONLY);
		auto* header = (CleanClusterListPass::OutputHeader*)data;
		auto globalActiveClusterCount = header->globalActiveClusterCount;
		auto* cleanedClusterList = (unsigned*)((char*)data + sizeof(CleanClusterListPass::OutputHeader));
	output->unmap();
}


nex::AABB nex::ProbeCluster::main(const glm::uvec3& gl_WorkGroupID, 
	const glm::uvec3& gl_NumWorkGroups, 
	const glm::vec2& zNearFar, 
	const glm::uvec4& tileSizes,
	const glm::mat4& invProj, 
	const glm::vec2& screenDimension)
{
	using namespace glm;

	//Eye position is zero in view space
	const vec3 eyePos = vec3(0.0);

	//Per tile variables

	// How many pixels in x does a square tile use
	uint tileSizePx = tileSizes[3];

	vec2 tileSizeRelative = 1.0f / vec2(gl_NumWorkGroups);

	// Linear ID of the thread/cluster
	uint clusterIndex = gl_WorkGroupID.x +
		gl_WorkGroupID.y * gl_NumWorkGroups.x +
		gl_WorkGroupID.z * (gl_NumWorkGroups.x * gl_NumWorkGroups.y);

	//Calculating the min and max point in screen space with origin at bottom(!) left corner
	vec4 maxScreenSpace = vec4(vec2(tileSizeRelative.x * (gl_WorkGroupID.x + 1), tileSizeRelative.y * (gl_WorkGroupID.y + 1)), -1.0, 1.0); // Top Right
	vec4 minScreenSpace = vec4(gl_WorkGroupID.x * tileSizeRelative.x, gl_WorkGroupID.y * tileSizeRelative.y, -1.0, 1.0); // Bottom left

	//Pass min and max to view space
	vec3 maxViewSpace = screen2View(maxScreenSpace, invProj, screenDimension);
	vec3 minViewSpace = screen2View(minScreenSpace, invProj, screenDimension);

	//Near and far values of the cluster in view space
	float clusterNear = zNearFar.x * pow(zNearFar.y / zNearFar.x, gl_WorkGroupID.z / float(gl_NumWorkGroups.z));
	float clusterFar = zNearFar.x * pow(zNearFar.y / zNearFar.x, (gl_WorkGroupID.z + 1) / float(gl_NumWorkGroups.z));

	const auto range = zNearFar.y - zNearFar.x;
	//clusterNear = zNearFar.x + gl_WorkGroupID.z * range / float(gl_NumWorkGroups.z);
	//clusterFar = zNearFar.x + (gl_WorkGroupID.z + 1) * range / float(gl_NumWorkGroups.z);

	//Finding the 4 intersection points made from the maxPoint to the cluster near/far plane
	vec3 minNear = lineIntersectionToZPlane(eyePos, minViewSpace, clusterNear);
	vec3 minFar = lineIntersectionToZPlane(eyePos, minViewSpace, clusterFar);
	vec3 maxNear = lineIntersectionToZPlane(eyePos, maxViewSpace, clusterNear);
	vec3 maxFar = lineIntersectionToZPlane(eyePos, maxViewSpace, clusterFar);

	vec3 minAABB = minVec(minVec(minNear, minFar), minVec(maxNear, maxFar));
	vec3 maxAABB = maxVec(maxVec(minNear, minFar), maxVec(maxNear, maxFar));

	return { minAABB, maxAABB };
}


//Creates a line from the eye to the screenpoint, then finds its intersection
//With a z oriented plane located at the given distance to the origin
glm::vec3 nex::ProbeCluster::lineIntersectionToZPlane(const glm::vec3& firstPoint, glm::vec3& secondPoint, float zValueViewSpace) {

	using namespace glm;

	//Because this is a Z based normal this is fixed
	vec3 normal = vec3(0.0, 0.0, -1.0);

	vec3 lineDirection = secondPoint - firstPoint;

	// Computing the intersection length for the line and the plane
	// For formula see 'Mathematics for 3D Game Programming and Computer Graphics', p.99, Eric Lengyel
	// Note: dot(normal, lineDirection) == -lineDirection.z since normal.x == normal.y == 0
	// and normal.z == -1.0!
	// Anologously we get dot(normal, firstPoint) == -firstPoint.z 
	//float t = -(dot(normal, firstPoint) + zValueViewSpace) / dot(normal, lineDirection);
	float t = (-firstPoint.z + zValueViewSpace) / lineDirection.z;

	//Computing the actual xyz position of the point along the line
	vec3 result = firstPoint + t * lineDirection;

	return result;
}

glm::vec4 nex::ProbeCluster::clipToView(const glm::vec4& clip, const glm::mat4& invProj) {

	using namespace glm;

	//View space transform
	vec4 view = invProj * clip;

	//Perspective division    
	return view / view.w;
}

glm::vec4 nex::ProbeCluster::screen2View(const glm::vec4& screen, const glm::mat4& invProj, const glm::vec2& screenDimension) {

	using namespace glm;

	//Convert to NDC
	vec2 texCoord = vec2(screen);// / screenDimension;

	//Convert to clipSpace
	vec4 clip = vec4(texCoord.x * 2.0 - 1.0, texCoord.y * 2.0 - 1.0, screen.z, screen.w);

	return clipToView(clip, invProj);
}


nex::gui::ProbeClusterView::ProbeClusterView(std::string title, 
	MainMenuBar* menuBar, 
	Menu* menu, 
	ProbeCluster* cluster, 
	PerspectiveCamera* activeCamera,
	nex::Window* window) :
	MenuWindow(std::move(title), menuBar, menu), mCluster(cluster), mActiveCamera(activeCamera), mWindow(window),
	mClusterSize(glm::uvec3(1))
{

}

void nex::gui::ProbeClusterView::drawSelf()
{

	auto& camera = mCluster->getCamera();
	auto position = camera.getPosition();
	auto fovY = glm::degrees(camera.getFovY());
	auto aspect = camera.getAspectRatio();
	auto zNear = camera.getNearDistance();
	auto zFar = camera.getFarDistance();
	auto look = camera.getLook();
	auto up = camera.getUp();

	
	if (ImGui::DragFloat3("Start position", (float*)& position)) {
		camera.setPosition(position, true);
	}

	if (ImGui::DragFloat3("Look direction", (float*)& look, 0.1f, -1.0f, 1.0f)) {
		camera.setLook(look);
	}

	if (ImGui::DragFloat3("Up direction", (float*)& up, 0.1f, -1.0f, 1.0f)) {
		camera.setUp(up);
	}
	
	if (ImGui::DragFloat("Vertical FOV", &fovY)) {
		camera.setFovY(glm::radians(fovY));
	}

	if (ImGui::DragFloat("Aspect ratio", &aspect)) {
		camera.setAspectRatio(aspect);
	}

	if (ImGui::DragFloat("Near plane", &zNear, 0.1f, 0.0f, FLT_MAX)) {
		camera.setNearDistance(zNear);
	}

	if (ImGui::DragFloat("Far plane", &zFar, 0.1f, 0.0f, FLT_MAX)) {
		camera.setFarDistance(zFar);
	}

	if (ImGui::Button("Align to active camera")) {
		camera.setPosition(mActiveCamera->getPosition(), true);
		camera.setFovY(mActiveCamera->getFovY());
		camera.setAspectRatio(mActiveCamera->getAspectRatio());
		camera.setNearDistance(mActiveCamera->getNearDistance());
		camera.setFarDistance(mActiveCamera->getFarDistance());
		camera.setLook(mActiveCamera->getLook());
		camera.setUp(mActiveCamera->getUp());
	}



	if (ImGui::Button("Create frustum")) {
		camera.update();
		mCluster->generate(camera.getFrustumWorld());
	}

	ImGui::Dummy(ImVec2(0, 10));
	nex::gui::Separator(2.0f);
	ImGui::Dummy(ImVec2(0, 10));
	ImGui::Text("Cluster element:");


	ImGui::DragFloat("X offset", &mClusterElement.xOffset, 0.1f, 0.0f, 1.0f, "%.5f");
	ImGui::DragFloat("Y offset", &mClusterElement.yOffset, 0.1f, 0.0f, 1.0f, "%.5f");
	ImGui::DragFloat("Z offset", &mClusterElement.zOffset, 0.1f, 0.0f, 1.0f, "%.5f");

	ImGui::DragFloat("width", &mClusterElement.width, 0.1f, 0.0f, 1.0f, "%.5f");
	ImGui::DragFloat("height", &mClusterElement.height, 0.1f, 0.0f, 1.0f, "%.5f");
	ImGui::DragFloat("depth", &mClusterElement.depth, 0.1f, 0.0f, 1.0f, "%.5f");

	if (ImGui::Button("Create cluster element")) {
		mCluster->generateClusterElement(mClusterElement);
	}

	ImGui::Dummy(ImVec2(0, 10));
	nex::gui::Separator(2.0f);
	ImGui::Dummy(ImVec2(0, 10));
	ImGui::Text("Cluster:");


	ImGui::DragScalar("X size", ImGuiDataType_U32 , &mClusterSize.x, 1.0f);
	ImGui::DragScalar("Y size", ImGuiDataType_U32, &mClusterSize.y, 1.0f);
	ImGui::DragScalar("Z size", ImGuiDataType_U32, &mClusterSize.z, 1.0f);

	if (ImGui::Button("Create cluster")) {
		mCluster->generateCluster(mClusterSize, mWindow->getFrameBufferWidth(), mWindow->getFrameBufferHeight());
	}
}

std::vector<nex::ClusterAABB>& nex::CullEnvironmentLightsCsCpuShader::getClusters()
{
	return clusters;
}

std::vector<nex::EnvironmentLight>& nex::CullEnvironmentLightsCsCpuShader::getEnvironmentLights()
{
	return environmentLights;
}

glm::uint nex::CullEnvironmentLightsCsCpuShader::getGlobalLightIndexCount()
{
	return globalIndexCount;
}

std::vector<glm::uint>& nex::CullEnvironmentLightsCsCpuShader::getGlobalLightIndexList()
{
	return globalLightIndexList;
}

std::vector<nex::ClusterLightGrid>& nex::CullEnvironmentLightsCsCpuShader::getLightGrids()
{
	return lightGrids;
}

void nex::CullEnvironmentLightsCsCpuShader::initInstance(const glm::uvec3& gl_NumWorkGroups, 
	const glm::uvec3& gl_GlobalInvocationID, 
	const glm::uvec3& gl_LocalInvocationID)
{
	this->gl_NumWorkGroups = gl_NumWorkGroups;
	this->gl_GlobalInvocationID = gl_GlobalInvocationID;
	this->gl_LocalInvocationID = gl_LocalInvocationID;

	this->gl_LocalInvocationIndex = gl_LocalInvocationID.z * gl_WorkGroupSize.x * gl_WorkGroupSize.y +
		gl_LocalInvocationID.y * gl_WorkGroupSize.x +
		gl_LocalInvocationID.x;

	sharedLights.resize(LOCAL_SIZE_X * LOCAL_SIZE_Y * LOCAL_SIZE_Z);
}

void nex::CullEnvironmentLightsCsCpuShader::test0()
{

	CullEnvironmentLightsCsCpuShader shader;
	shader.setGlobalIndexCount(0);
	shader.setViewMatrix(glm::mat4());

	const auto LIGHT_COUNT = 1;
	shader.getGlobalLightIndexList().resize(LIGHT_COUNT * MAX_VISIBLES_LIGHTS);
	auto& lights = shader.getEnvironmentLights();

	lights.resize(LIGHT_COUNT);

	// first light
	lights[0].enabled = true;
	lights[0].position = glm::vec4(0.0f);
	lights[0].sphereRange = 10.0f;
	lights[0].usesBoundingBox = false;
	lights[0].minWorld = glm::vec4(-1.0f);
	lights[0].maxWorld = glm::vec4(1.0f);


	const auto BATCH_SIZE = LOCAL_SIZE_X * LOCAL_SIZE_Y * LOCAL_SIZE_Z;
	const auto numWorkGroups = glm::uvec3(2,2,2);

	auto& clusters = shader.getClusters();
	clusters.resize(BATCH_SIZE * numWorkGroups.x * numWorkGroups.y * numWorkGroups.z);
	shader.getLightGrids().resize(clusters.size());
	for (auto& cluster : clusters) {
		cluster.minView = cluster.minWorld = glm::vec4(-10.0f);
		cluster.maxView = cluster.maxWorld = glm::vec4(10.0f);
	}

	shader.initInstance(numWorkGroups, glm::uvec3(0,1,1), glm::uvec3(8,2,3));
	shader.main();
}

void nex::CullEnvironmentLightsCsCpuShader::main() const
{
	using namespace glm;

	globalIndexCount = 0;
	uint threadCount = gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;
	uint lightCount = environmentLights.size();
	uint numBatches = (lightCount + threadCount - 1) / threadCount;

	uvec3 globalSize = gl_NumWorkGroups * gl_WorkGroupSize;
	uint globalInvocationIndex = gl_GlobalInvocationID.z * globalSize.x * globalSize.y +
		gl_GlobalInvocationID.y * globalSize.x +
		gl_GlobalInvocationID.x;

	uint visibleLightCount = 0;
	uint visibleLightIndices[MAX_VISIBLES_LIGHTS];

	for (uint batch = 0; batch < numBatches; ++batch) {
		//uint lightIndex = batch * threadCount + gl_LocalInvocationIndex;


		for (uint index = 0; index < threadCount; ++index) {
			uint lightIndex = batch * threadCount + index;
			//Prevent overflow by clamping to last light which is always null
			lightIndex = min(lightIndex, lightCount);
			if (lightIndex < lightCount) {
				sharedLights[index] = environmentLights[lightIndex];
			}
			else {
				sharedLights[index].enabled = false;
			}
		}

		//Prevent overflow by clamping to last light which is always null
		//lightIndex = min(lightIndex, lightCount - 1);

		//Populating shared light array
		//sharedLights[gl_LocalInvocationIndex] = environmentLights[lightIndex];
		//barrier();

		//Iterating within the current batch of lights
		for (uint light = 0; light < threadCount && (visibleLightCount < MAX_VISIBLES_LIGHTS); ++light) {
			if (sharedLights[light].enabled == 1) {

				if (sharedLights[light].usesBoundingBox) {
					if (testAABBWorld(light, globalInvocationIndex)) {
						visibleLightIndices[visibleLightCount] = batch * threadCount + light;
						visibleLightCount += 1;
					}

				}
				else {
					if (testSphereAABB(light, globalInvocationIndex)) {
						visibleLightIndices[visibleLightCount] = batch * threadCount + light;
						visibleLightCount += 1;
					}
				}
			}
		}
	}

	//We want all thread groups to have completed the light tests before continuing
	//barrier();

	//uint offset = atomicAdd(globalIndexCount, visibleLightCount);
	uint offset = globalIndexCount;
	globalIndexCount += visibleLightCount;
	

	for (uint i = 0; i < visibleLightCount; ++i) {
		globalLightIndexList[offset + i] = visibleLightIndices[i];
	}

	lightGrids[globalInvocationIndex].offset = offset;
	lightGrids[globalInvocationIndex].count = visibleLightCount;
}

void nex::CullEnvironmentLightsCsCpuShader::setGlobalIndexCount(glm::uint count)
{
	globalIndexCount = count;
}

void nex::CullEnvironmentLightsCsCpuShader::setViewMatrix(const glm::mat4& mat)
{
	viewMatrix = mat;
}

float nex::CullEnvironmentLightsCsCpuShader::sqDistPointAABB(glm::vec3 point, glm::uint clusterID) const
{
	float sqDist = 0.0;
	ClusterAABB currentCell = clusters[clusterID];
	//cluster[clusterID].maxPoint[3] = clusterID;
	for (int i = 0; i < 3; ++i) {
		float v = point[i];

		if (v < currentCell.minView[i])
		{
			float dist = currentCell.minView[i] - v;
			sqDist += dist * dist;
		}
		if (v > currentCell.maxView[i])
		{
			float dist = v - currentCell.maxView[i];
			sqDist += dist * dist;
		}
	}

	return sqDist;
}

bool nex::CullEnvironmentLightsCsCpuShader::testAABBWorld(glm::uint light, glm::uint clusterID) const
{
	ClusterAABB currentCell = clusters[clusterID];
	EnvironmentLight envLight = environmentLights[light];

	return (currentCell.minWorld.x <= envLight.maxWorld.x && currentCell.maxWorld.x >= envLight.minWorld.x) &&
		(currentCell.minWorld.y <= envLight.maxWorld.y && currentCell.maxWorld.y >= envLight.minWorld.y) &&
		(currentCell.minWorld.z <= envLight.maxWorld.z && currentCell.maxWorld.z >= envLight.minWorld.z);
}

bool nex::CullEnvironmentLightsCsCpuShader::testSphereAABB(glm::uint light, glm::uint clusterID) const
{
	float radius = sharedLights[light].sphereRange;
	glm::vec3 center = glm::vec3(viewMatrix * sharedLights[light].position);
	float squaredDistance = sqDistPointAABB(center, clusterID);

	return squaredDistance <= (radius * radius);
}