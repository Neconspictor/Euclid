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

class nex::ProbeCluster::GenerateClusterPass : public nex::ComputePass {
public:

	struct AABB {
		glm::vec4 min;
		glm::vec4 max;
	};

	struct Constants {
		glm::mat4 invProj;
		glm::vec4 zNearFar; // near and far plane in view space; z and w component unused
	};

	GenerateClusterPass() : 
		ComputePass(Shader::createComputeShader("cluster/clusters_cs.glsl")) 
	{

	}

	static unsigned getClusterAABBBinding() {
		return 0;
	}

	static unsigned getConstantsBinding() {
		return 1;
	}
};


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

	void resizeBuffer(const ClusterSize& clusterSize) {
		const auto flattenedSize = clusterSize.xSize * clusterSize.ySize * clusterSize.zSize;
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


nex::ProbeCluster::ProbeCluster(Scene* scene) : mScene(scene), 
mPass(std::make_unique<SimpleColorPass>()),
mTechnique(std::make_unique<Technique>(mPass.get())), 
mMaterial(std::make_unique<Material>(mTechnique.get())),
mGenerateClusterShader(std::make_unique<GenerateClusterPass>()),
mConstantsBuffer(std::make_unique<ShaderStorageBuffer>(GenerateClusterPass::getConstantsBinding(), 
	sizeof(GenerateClusterPass::Constants), 
	nullptr, 
	GpuBuffer::UsageHint::DYNAMIC_COPY)),
mClusterAABBBuffer(std::make_unique<ShaderStorageBuffer>(GenerateClusterPass::getClusterAABBBinding(), 
	0, // Note: we dynamically resize the buffer 
	nullptr, 
	GpuBuffer::UsageHint::DYNAMIC_COPY)),
mCollectClustersPass(std::make_unique<CollectClustersPass>()),
mCleanClusterListPass(std::make_unique<CleanClusterListPass>())
{
	mPass->bind();
	mPass->setColor(glm::vec4(2.0f, 0.0f, 0.0f, 1.0f));

	auto& state = mMaterial->getRenderState();
	state.fillMode = FillMode::LINE;
	state.doCullFaces = false;
	state.isTool = true;
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

void nex::ProbeCluster::generateCluster(const ClusterSize& clusterSize)
{
	//generateClusterCpuTest(clusterSize); return;
	generateClusterGpu(clusterSize); return;
	mCamera.update();
	ClusterElement elem;
	auto container = std::make_unique<StaticMeshContainer>();

	elem.width = 1.0f / (float) clusterSize.xSize;
	elem.height = 1.0f / (float) clusterSize.ySize;
	elem.depth = 1.0f / (float) clusterSize.zSize;
	const auto viewInv = inverse(mCamera.getView());

	const auto middleDistance = (mCamera.getFarDistance() + mCamera.getNearDistance()) / 2.0f;
	const auto lookVecView = glm::vec3(0,0, Camera::getViewSpaceZfromDistance(1.0f));
	const auto middleTrans = glm::translate(glm::mat4(), -middleDistance * lookVecView);


	for (size_t x = 0; x < clusterSize.xSize; ++x ) {
		for (size_t y = 0; y < clusterSize.ySize; ++y) {
			for (size_t z = 0; z < clusterSize.zSize; ++z) {

				elem.xOffset = x * elem.width;
				elem.yOffset = y * elem.height;
				elem.zOffset = z * elem.depth;

				auto frustumView = mCamera.calcClusterElementViewSpace(elem.xOffset, elem.yOffset, elem.zOffset, elem.depth, elem.width, elem.height);
				
				//const auto frustumWorld = frustumView * viewInv;
				//auto mesh = std::make_unique<FrustumMesh>(frustumWorld);
				frustumView = middleTrans * frustumView;
				auto mesh = std::make_unique<MeshAABB>(frustumView.calcAABB());
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

void nex::ProbeCluster::generateClusterCpuTest(const ClusterSize& clusterSize)
{
	mCamera.update();
	ClusterElement elem;
	auto container = std::make_unique<StaticMeshContainer>();

	elem.width = 1.0f / (float)clusterSize.xSize;
	elem.height = 1.0f / (float)clusterSize.ySize;
	elem.depth = 1.0f / (float)clusterSize.zSize;
	const auto viewInv = inverse(mCamera.getView());

	const auto middleDistance = (mCamera.getFarDistance() + mCamera.getNearDistance()) / 2.0f;
	const auto lookVecView = glm::vec3(0, 0, Camera::getViewSpaceZfromDistance(1.0f));
	const auto middleTrans = glm::translate(glm::mat4(), -middleDistance * lookVecView);

	const glm::vec3 workGroupSize(clusterSize.xSize, clusterSize.ySize, clusterSize.zSize);
	const glm::vec2 zNearFar (Camera::getViewSpaceZfromDistance(mCamera.getNearDistance()), 
		Camera::getViewSpaceZfromDistance(mCamera.getFarDistance()));
	const glm::uvec4 tileSizes(1920 / clusterSize.xSize);
	const auto invProj = inverse(mCamera.getProjectionMatrix());
	const glm::vec2 screenDimension(1920, 1080);


	for (size_t x = 0; x < clusterSize.xSize; ++x) {
		for (size_t y = 0; y < clusterSize.ySize; ++y) {
			for (size_t z = 0; z < clusterSize.zSize; ++z) {

				elem.xOffset = x * elem.width;
				elem.yOffset = y * elem.height;
				elem.zOffset = z * elem.depth;

				//auto frustumView = mCamera.calcClusterElementViewSpace(elem.xOffset, elem.yOffset, elem.zOffset, elem.depth, elem.width, elem.height);

				//const auto frustumWorld = frustumView * viewInv;
				//auto mesh = std::make_unique<FrustumMesh>(frustumWorld);
				//frustumView = middleTrans * frustumView;
				auto box = main(glm::vec3(x,y,z), workGroupSize, zNearFar,tileSizes, invProj,
					screenDimension);
				box.min = glm::vec3(middleTrans * glm::vec4(box.min, 1.0f));
				box.max = glm::vec3(middleTrans * glm::vec4(box.max, 1.0f));

				auto mesh = std::make_unique<MeshAABB>(box);


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

void nex::ProbeCluster::generateClusterGpu(const ClusterSize& clusterSize)
{
	mCamera.update();
	const auto viewInv = inverse(mCamera.getView());
	const auto middleDistance = (mCamera.getFarDistance() + mCamera.getNearDistance()) / 2.0f;
	const auto lookVecView = glm::vec3(0, 0, Camera::getViewSpaceZfromDistance(1.0f));
	const auto middleTrans = glm::translate(glm::mat4(), -middleDistance * lookVecView);

	auto container = std::make_unique<StaticMeshContainer>();

	mGenerateClusterShader->bind();

	mConstantsBuffer->bindToTarget();
	mClusterAABBBuffer->bindToTarget();

	// update buffers
	const auto flattedClusterSize = clusterSize.xSize* clusterSize.ySize* clusterSize.zSize;
	mClusterAABBBuffer->resize(sizeof(GenerateClusterPass::AABB) * flattedClusterSize, nullptr, GpuBuffer::UsageHint::DYNAMIC_COPY);
	GenerateClusterPass::Constants constants;
	constants.invProj = inverse(mCamera.getProjectionMatrix());
	constants.zNearFar = glm::vec4(Camera::getViewSpaceZfromDistance(mCamera.getNearDistance()), 
		Camera::getViewSpaceZfromDistance(mCamera.getFarDistance()), 0, 0);
	mConstantsBuffer->update(sizeof(GenerateClusterPass::Constants), &constants);
	
	// generate clusters
	mGenerateClusterShader->dispatch(clusterSize.xSize, clusterSize.ySize, clusterSize.zSize);


	// Readback the generated clusters
	auto* clusters = (GenerateClusterPass::AABB*)mClusterAABBBuffer->map(GpuBuffer::Access::READ_ONLY);
		for (unsigned i = 0; i < flattedClusterSize; ++i) {
			auto& cluster = clusters[i];

			AABB box = {glm::vec3(cluster.min), glm::vec3(cluster.max)};

			box.min = glm::vec3(middleTrans * glm::vec4(box.min, 1.0f));
			box.max = glm::vec3(middleTrans * glm::vec4(box.max, 1.0f));

			auto mesh = std::make_unique<MeshAABB>(box);
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
	vob->setPosition(vob->getPosition() + middlePoint);
	vob->updateTrafo(true);

	mScene->addVobUnsafe(std::move(vob), true);


	collectActiveClusterGpuTest(clusterSize, mCamera.getNearDistance(), mCamera.getFarDistance());
	cleanActiveClusterListGpuTest(clusterSize, mCollectClustersPass->getBuffer());
}

void nex::ProbeCluster::collectActiveClusterGpuTest(const ClusterSize& clusterSize, float zNearDistance, float zFarDistance)
{
	mCollectClustersPass->bind();
	
	mCollectClustersPass->resizeBuffer(clusterSize);
	auto* buffer = mCollectClustersPass->getBuffer();

	void* data = buffer->map(GpuBuffer::Access::READ_WRITE);
		auto* activeClusters = (CollectClustersPass::ActiveClusters*)data;
		activeClusters->numClusters = glm::uvec4(clusterSize.xSize, clusterSize.ySize, clusterSize.zSize, 0);

		auto logzFar_zNear = logf(zFarDistance / zNearDistance);
		activeClusters->constantsAB = glm::vec4(logzFar_zNear, logf(zNearDistance) * clusterSize.zSize / logzFar_zNear, 0.0f, 0.0f);
	
		auto* flagArray = (unsigned*)((char*)data + sizeof(CollectClustersPass::ActiveClusters));
		const auto flattenedSize = clusterSize.xSize * clusterSize.ySize * clusterSize.zSize;
		memset(flagArray, 0, flattenedSize * sizeof(unsigned));
	buffer->unmap();

	buffer->bindToTarget();
	mCollectClustersPass->dispatch(1920, 1080, 1);

	data = buffer->map(GpuBuffer::Access::READ_WRITE);
		activeClusters = (CollectClustersPass::ActiveClusters*)data;
		flagArray = (unsigned*)((char*)data + sizeof(CollectClustersPass::ActiveClusters));
	buffer->unmap();
}

void nex::ProbeCluster::cleanActiveClusterListGpuTest(const ClusterSize& clusterSize, ShaderStorageBuffer* activeClusters)
{
	mCleanClusterListPass->bind();


	auto* output = mCleanClusterListPass->getOutput();
	unsigned defaultGlobalActiveClusterCount = 0;
	const auto flattenedSize = clusterSize.xSize * clusterSize.ySize * clusterSize.zSize;
	output->resize(sizeof(unsigned) * (flattenedSize + 1), nullptr, GpuBuffer::UsageHint::STREAM_DRAW);
	output->update(sizeof(unsigned), &defaultGlobalActiveClusterCount);

	activeClusters->bindToTarget(mCleanClusterListPass->getInputBindingPoint());
	output->bindToTarget();

	mCleanClusterListPass->dispatch(clusterSize.xSize, clusterSize.ySize, clusterSize.zSize);

	void* data = output->map(GpuBuffer::Access::READ_ONLY);
		auto* header = (CleanClusterListPass::OutputHeader*)data;
		auto globalActiveClusterCount = header->globalActiveClusterCount;
		auto* cleanedClusterList = (unsigned*)((char*)data + sizeof(CleanClusterListPass::OutputHeader));
	output->unmap();
}


nex::AABB nex::ProbeCluster::main(const glm::vec3& gl_WorkGroupID, 
	const glm::vec3& gl_NumWorkGroups, 
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


nex::gui::ProbeClusterView::ProbeClusterView(std::string title, MainMenuBar* menuBar, Menu* menu, ProbeCluster* cluster, PerspectiveCamera* activeCamera) :
	MenuWindow(std::move(title), menuBar, menu), mCluster(cluster), mActiveCamera(activeCamera)
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


	ImGui::DragScalar("X size", ImGuiDataType_U64 , &mClusterSize.xSize, 1.0f);
	ImGui::DragScalar("Y size", ImGuiDataType_U64, &mClusterSize.ySize, 1.0f);
	ImGui::DragScalar("Z size", ImGuiDataType_U64, &mClusterSize.zSize, 1.0f);

	if (ImGui::Button("Create cluster")) {
		mCluster->generateCluster(mClusterSize);
	}
}