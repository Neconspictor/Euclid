#include <nex/shadow/CascadedShadow.hpp>
#include <nex/mesh/Mesh.hpp>
#include <nex/texture/RenderTarget.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <glm/gtc/matrix_transform.inl>
#include "nex/resource/FileSystem.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <nex/gui/ImGUI.hpp>
#include "nex/gui/ImGUI_Extension.hpp"
#include "imgui/imgui_internal.h"
#include "nex/texture/Attachment.hpp"
#include <nex/shadow/SceneNearFarComputePass.hpp>
#include <nex/material/Material.hpp>
#include <nex/texture/Image.hpp>
#include "ShadowMap.hpp"
#include <nex/renderer/Drawer.hpp>
#include <nex/mesh/MeshGroup.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/resource/ResourceLoader.hpp>

using namespace nex;


CascadedShadow::CascadedShadow(unsigned int cascadeWidth, unsigned int cascadeHeight, unsigned numCascades, const PCFFilter& pcf, float biasMultiplier, bool antiFlickerOn,
	float shadowStrength) :
	mCascadeWidth(cascadeWidth),
	mCascadeHeight(cascadeHeight),
	mShadowMapSize(std::min<unsigned>(cascadeWidth, cascadeHeight)),
	mAntiFlickerOn(antiFlickerOn),
	mPCF(pcf),
	mEnabled(true),
	mBiasMultiplier(biasMultiplier),
	mShadowStrength(shadowStrength),
	mDepthPass(std::make_unique<DepthPass>(numCascades, false)),
	mDepthPassBones(std::make_unique<DepthPass>(numCascades, true)),
	mDataComputePass(std::make_unique<CascadeDataPass>(numCascades)),
	mUseTightNearFarPlane(true),
	mSceneNearFarComputeShader(std::make_unique<SceneNearFarComputePass>()),
	mRenderTarget(cascadeWidth, cascadeHeight)
{
	mNumCascades = numCascades;
	mSplitDistances.resize(numCascades);
	mCascadeBoundCenters.resize(numCascades);

	// reset cascade data 
	for (int i = 0; i < mNumCascades; i++)
	{
		mCascadeBoundCenters[i] = glm::vec3(0.0f);
		mSplitDistances[i] = 0.0f;
		mCascadeData.cascadedSplits[i] = glm::vec4(0.0f);
		mCascadeData.scaleFactors[i] = glm::vec4(0.0f);
		mCascadeData.lightViewProjectionMatrices[i] = glm::mat4(0.0f);
	}

	resize(cascadeWidth, cascadeHeight);
}

CascadedShadow::~CascadedShadow() = default;

std::vector<std::string> CascadedShadow::generateCsmDefines() const
{
	std::vector<std::string> result;

	result.emplace_back(ShaderProgram::makeDefine("CSM_NUM_CASCADES", mNumCascades));
	result.emplace_back(ShaderProgram::makeDefine("CSM_SAMPLE_COUNT_X", mPCF.sampleCountX));
	result.emplace_back(ShaderProgram::makeDefine("CSM_SAMPLE_COUNT_Y", mPCF.sampleCountY));
	result.emplace_back(ShaderProgram::makeDefine("CSM_USE_LERP_FILTER", mPCF.useLerpFiltering));
	result.emplace_back(ShaderProgram::makeDefine("CSM_ENABLED", mEnabled));
	result.emplace_back(ShaderProgram::makeDefine("CSM_BIAS_MULTIPLIER", mBiasMultiplier));
	result.emplace_back(ShaderProgram::makeDefine("CSM_VISUALIZE_CASCADES", mVisualizeCascades));

	return result;
}

void nex::CascadedShadow::bind(const RenderContext& constants)
{
	//mDepthPass->bind();
	//mDepthPass->updateConstants(constants);

	mRenderTarget.bind();
	RenderBackend::get()->setViewPort(0, 0, mCascadeWidth, mCascadeHeight); //TODO move out of function
}

void CascadedShadow::begin(int cascadeIndex)
{
	mDepthPass->setCascadeIndexRaw(cascadeIndex);
	mDepthPassBones->setCascadeIndexRaw(cascadeIndex);

	auto* depth = mRenderTarget.getDepthAttachment();
	depth->layer = cascadeIndex;
	mRenderTarget.updateDepthAttachment();
	mRenderTarget.clear(RenderComponent::Depth);
}

void CascadedShadow::enable(bool enable, bool informObservers)
{
	mEnabled = enable;
	if (informObservers) informCascadeChanges();
}

Texture* CascadedShadow::getDepthTextureArray()
{
	return mRenderTarget.getDepthAttachment()->texture.get();
}

const Texture* CascadedShadow::getDepthTextureArray() const
{
	return mRenderTarget.getDepthAttachment()->texture.get();
}

void CascadedShadow::resize(unsigned cascadeWidth, unsigned cascadeHeight)
{
	mCascadeWidth = cascadeWidth;
	mCascadeHeight = cascadeHeight;
	mShadowMapSize = std::min<unsigned>(cascadeWidth, cascadeHeight);

	// reset cascade cound centers 
	for (auto i = 0; i < mNumCascades; i++)
	{
		mCascadeBoundCenters[i] = glm::vec3(0.0f);
		mSplitDistances[i] = 0.0f;
	}

	mDataComputePass->resetPrivateData();

	updateTextureArray();
}

nex::CascadedShadow::ChangedCallback::Handle nex::CascadedShadow::addChangedCallback(const ChangedCallback::Callback& callback)
{
	return mCallbacks.addCallback(callback);
}

void nex::CascadedShadow::removeChangedCallback(const ChangedCallback::Handle& handle)
{
	mCallbacks.removeCallback(handle);
}

void CascadedShadow::informCascadeChanges()
{
	for (auto& handle : mCallbacks.getCallbacks())
	{
		const auto& callback = *handle;
		callback(this);
	}
}

bool CascadedShadow::isEnabled() const
{
	return mEnabled;
}

void CascadedShadow::updateTextureArray()
{
	TextureDesc data;
	data.internalFormat = InternalFormat::DEPTH16;
	data.minFilter = TexFilter::Nearest; // IMPORTANT: Linear filter produces ugly artifacts when using PCF filtering
	data.magFilter = TexFilter::Nearest; // IMPORTANT: Linear filter produces ugly artifacts when using PCF filtering
	data.wrapR = data.wrapS = data.wrapT = UVTechnique::ClampToBorder;
	data.borderColor = glm::vec4(1.0f);
	//data.useDepthComparison = true;
	data.compareFunction = CompFunc::LESS;
	data.useSwizzle = true;
	data.swizzle = { Channel::RED, Channel::RED, Channel::RED, Channel::ALPHA };

	RenderAttachment depth;
	depth.type = RenderAttachmentType::DEPTH;
	depth.target = TextureTarget::TEXTURE2D_ARRAY;
	depth.texture = std::make_unique<Texture2DArray>(mCascadeWidth, mCascadeHeight, mNumCascades, data, nullptr);

	mRenderTarget.bind();
	mRenderTarget.useDepthAttachment(std::move(depth));
	mRenderTarget.finalizeAttachments();
	mRenderTarget.assertCompletion();
}

void nex::CascadedShadow::frameUpdateTightNearFarPlane(const Camera& camera, const glm::vec3 & lightDirection, nex::ShaderStorageBuffer * minMaxOutputBuffer)
{
	//mDataComputeShader->getSharedOutput()->unbind();
	mDataComputePass->bind();
	mDataComputePass->setUseAntiFlickering(mAntiFlickerOn);
	mDataComputePass->useDistanceInputBuffer(minMaxOutputBuffer);

	CascadeDataPass::Input constantInput;

	constantInput.lightDirection = glm::vec4(lightDirection, 0.0f);
	constantInput.nearFarPlane = glm::vec4(camera.getNearDistance(), camera.getFarDistance(), 0.0f, 0.0f);
	constantInput.shadowMapSize = glm::vec4(mShadowMapSize, 0.0f, 0.0f, 0.0f);
	constantInput.cameraPostionWS = glm::vec4(camera.getPosition(), 0.0f);
	constantInput.cameraLook = glm::vec4(camera.getLook(), 0.0f);
	constantInput.viewMatrix = camera.getView();
	constantInput.projectionMatrix = camera.getProjectionMatrix();

	mDataComputePass->mInputBuffer->bindToTarget();
	mDataComputePass->update(constantInput);

	mDataComputePass->mPrivateOutput->bindToTarget();

	auto* cs = mDataComputePass->getSharedOutput();
	cs->bindToTarget();

	mDataComputePass->dispatch(1, 1, 1);
}

void CascadedShadow::frameUpdateNoTightNearFarPlane(const Camera& camera, const glm::vec3& lightDirection,
	const glm::vec2& minMaxPositiveZ)
{
	const float minDistance = 0.0f;//minMaxPositiveZ.x / (minMaxPositiveZ.y - minMaxPositiveZ.x);
	//const Frustum& frustum = camera->getFrustum(ProjectionMode::Perspective);
	//const float cameraNearPlaneVS = frustum.nearPlane;
	calcSplitSchemes(minMaxPositiveZ);
	mGlobal = calcShadowSpaceMatrix(camera, lightDirection);

	//Note: We need the inverse view matrix; but the transpose is sufficient in this case
	const glm::mat3 shadowOffsetMatrix = glm::mat3(glm::transpose(mGlobal.shadowView));


	for (unsigned int cascadeIterator = 0; cascadeIterator < mNumCascades; ++cascadeIterator)
	{
		// the far plane of the previous cascade is the near plane of the current cascade
		//const float nearPlane = cascadeIterator == 0 ? cameraNearPlaneVS : mCascadeData.cascadedFarPlanes[cascadeIterator - 1].x;
		//const float farPlane = mCascadeData.cascadedFarPlanes[cascadeIterator].x;
		const float nearSplitDistance = minMaxPositiveZ.x + (cascadeIterator == 0 ? minDistance : mSplitDistances[cascadeIterator - 1]);
		const float farSplitDistance = minMaxPositiveZ.x + mSplitDistances[cascadeIterator];


		glm::vec3 cascadeCenterShadowSpace;
		float scale;

		if (mAntiFlickerOn)
		{

			// To avoid anti flickering we need to make the transformation invariant to camera rotation and translation
			// By encapsulating the cascade frustum with a sphere we achive the rotation invariance
			const auto boundingSphere = extractFrustumBoundSphere(camera, nearSplitDistance, farSplitDistance);
			const float radius = boundingSphere.radius;
			const glm::vec3& frustumCenterWS = boundingSphere.center;

			// Only update the cascade bounds if it moved at least a full pixel unit
			// This makes the transformation invariant to translation
			glm::vec3 offset;
			if (cascadeNeedsUpdate(mGlobal.shadowView, cascadeIterator, frustumCenterWS, mCascadeBoundCenters[cascadeIterator], radius, &offset))
			{
				// To avoid flickering we need to move the bound center in full units
				// NOTE: we don't want translation affect the offset!
				glm::vec3 offsetWS = shadowOffsetMatrix * offset;
				mCascadeBoundCenters[cascadeIterator] += offsetWS;
				//mCascadeBoundCenters[cascadeIterator] = glm::vec4(frustumCenterWS + offset, 1.0f);
			}

			// Get the cascade center in shadow space
			cascadeCenterShadowSpace = glm::vec3(mGlobal.worldToShadowSpace * glm::vec4(mCascadeBoundCenters[cascadeIterator], 1.0f));
			//cascadeCenterShadowSpace = glm::vec3(mGlobal.worldToShadowSpace * glm::vec4(frustumCenterWS, 1.0f));

			// Update the scale from shadow to cascade space
			scale = mGlobal.radius / radius; //2.0f * mGlobal.radius /
		}
		else
		{

			glm::vec3 frustumPointsWS[8];
			camera.calcFrustumCornersWS(frustumPointsWS, nearSplitDistance, farSplitDistance);
			glm::vec3 maxExtents = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
			glm::vec3 minExtents = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);

			for (auto i = 0; i < 8; ++i)
			{
				glm::vec3 pointInShadowSpace = mGlobal.worldToShadowSpace * glm::vec4(frustumPointsWS[i], 1.0f);
				minExtents = minVec(minExtents, pointInShadowSpace);
				maxExtents = maxVec(maxExtents, pointInShadowSpace);
			}

			cascadeCenterShadowSpace = 0.5f * (minExtents + maxExtents);
			scale = 2.0f / std::max<float>(maxExtents.x - minExtents.x, maxExtents.y - minExtents.y);
		}


		// Update the translation from shadow to cascade space
		auto cascadeTrans = glm::translate(glm::mat4(1.0f), glm::vec3(cascadeCenterShadowSpace.x, cascadeCenterShadowSpace.y, 0.0f));
		auto cascadeTransInv = glm::translate(glm::mat4(1.0f), glm::vec3(-cascadeCenterShadowSpace.x, -cascadeCenterShadowSpace.y, 0.0f));
		

		auto cascadeScale = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, 1.0f));

		//Store the split distances and the relevant matrices
		mCascadeData.lightViewProjectionMatrices[cascadeIterator] = cascadeScale * cascadeTransInv * mGlobal.worldToShadowSpace;//cascadeProjMatrix * cascadeViewMatrix;
		mCascadeData.scaleFactors[cascadeIterator].x = scale;
	}

	//mDataComputePass->getSharedOutput()->update(sizeof(CascadeData), &mCascadeData);
}


CascadedShadow::GlobalShadow CascadedShadow::calcShadowSpaceMatrix(const Camera& camera, const glm::vec3& lightDirection)
{
	const auto nearDistance = camera.getNearDistance();
	const auto farDistance = camera.getFarDistance();
	const float cascadeTotalRange = farDistance - nearDistance;
	const auto shadowBounds = extractFrustumBoundSphere(camera, nearDistance, farDistance);

	// Find the view matrix
	const glm::vec3 cameraFrustumCenterWS = camera.getPosition() + camera.getLook() * cascadeTotalRange * 0.5f;
	const glm::vec3 lookAt = cameraFrustumCenterWS + normalize(lightDirection) * farDistance;

	const glm::vec3 lightPos = cameraFrustumCenterWS + normalize(lightDirection) * shadowBounds.radius;

	//glm::vec3 upVec = glm::normalize(glm::cross(lightDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
	glm::vec3 upVec = glm::normalize(glm::cross(lightDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
	if (abs(dot(upVec, lightDirection)) < 0.0001f)
	{
		upVec = glm::normalize(glm::cross(lightDirection, glm::vec3(0.0f, 0.0f, -1.0f)));
	}

	const glm::mat4 shadowView = glm::lookAt(cameraFrustumCenterWS, lightPos, upVec);


	// Get the bounds for the shadow space

	//const auto shadowProj = glm::ortho(-shadowBounds.radius, shadowBounds.radius,
	//	-shadowBounds.radius, shadowBounds.radius,
	//	-shadowBounds.radius, shadowBounds.radius);

	const auto shadowProj = glm::ortho(-shadowBounds.radius, shadowBounds.radius,
		-shadowBounds.radius, shadowBounds.radius,
		-shadowBounds.radius, shadowBounds.radius);

	//orthoNO

	// The combined transformation from world to shadow space
	GlobalShadow result;
	result.worldToShadowSpace = shadowProj * shadowView;
	result.shadowView = shadowView;
	result.radius = shadowBounds.radius;
	return result;
}

void CascadedShadow::calcSplitSchemes(const glm::vec2& minMaxPositiveZ)
{
	calcSplitDistances(minMaxPositiveZ.y - minMaxPositiveZ.x, minMaxPositiveZ);

	const float nearClip = minMaxPositiveZ.x;

	// We calculate the splitting planes of the view frustum by using an algorithm 
	for (unsigned int i = 0; i < mNumCascades; ++i)
	{
		//nearClip +
		mCascadeData.cascadedSplits[i].x = (nearClip + mSplitDistances[i]);// *clipRange);
	}
}

void CascadedShadow::calcSplitDistances(float range, const glm::vec2& minMaxPositiveZ)
{
	/*//Between 0 and 1, change in order to see the results
	const float lambda = 1.0f;
	//Between 0 and 1, change these to check the results
	const float minDistance = 0.0f;
	const float maxDistance = 1.0f;*/


	/*const float nearClip = minMaxPositiveZ.x;//frustum.nearPlane;
	//const float farClip = minMaxPositiveZ.y;// frustum.farPlane;
	const float clipRange = max(minMaxPositiveZ.y - nearClip, 10.0f);

	const float minZ = nearClip + minDistance * clipRange;
	const float maxZ = nearClip + maxDistance * clipRange;

	const float range = maxZ - minZ;
	const float ratio = maxZ / minZ;*/

	float step = range / (float)mNumCascades;

	// We calculate the splitting planes of the view frustum by using an algorithm 
	for (unsigned int i = 0; i < mNumCascades; ++i)
	{
		//const float p = (i + 1) / static_cast<float>(mCascadeData.numCascades);
		//const float log = minZ * std::pow(ratio, p);
		//const float uniform = minZ + range * p;
		//const float d = lambda * (log - uniform) + uniform;
		//mSplitDistances[i] = (d - nearClip);
		mSplitDistances[i] = ((i + 1)*step);


		/*switch (i) {
		case 0: mSplitDistances[i] = 5.0f + nearClip;
			break;
		case 1: mSplitDistances[i] = 20.0f + nearClip;
			break;
		case 2: mSplitDistances[i] = 40.0f + nearClip;
			break;
		case 3: mSplitDistances[i] = 60.0f + nearClip;
			break;
		case 4: mSplitDistances[i] = 150.0f + nearClip;
			break;
		case 5: mSplitDistances[i] = 200.0f + nearClip;
			break;
		case 6: mSplitDistances[i] = 300.0f + nearClip;
			break;
		case 7: mSplitDistances[i] = 400.0f + nearClip;
			break;
		default: break;
		}

		mSplitDistances[i] *= 0.5f;*/

		// do some rounding for fighting numerical issues
		// This helps to reduce flickering
		mSplitDistances[i] = std::ceil(mSplitDistances[i] * 32.0f) / 32.0f;
	}

	if (mUseLogarithmicSplits) {
		for (int i = 0; i < mNumCascades - 1; ++i) {

			float splitRange = mSplitDistances[i];
			if (i > 0)splitRange = splitRange - mSplitDistances[i - 1];

			mSplitDistances[i] *= log(splitRange) / log(range);
		}
	}
}

CascadedShadow::BoundingSphere CascadedShadow::extractFrustumBoundSphere(const Camera& camera, float nearSplitDistance, float farSplitDistance)
{
	glm::vec3 frustumCornersWS[8];
	camera.calcFrustumCornersWS(frustumCornersWS, nearSplitDistance, farSplitDistance);
	// calc center of the frustum
	glm::vec3 frustumCenter = glm::vec3(0.0f);
	for (unsigned int i = 0; i < 8; ++i)
		frustumCenter += frustumCornersWS[i];
	frustumCenter /= 8.0f;

	/*glm::vec3 minVec = frustumCornersWS[0];
	glm::vec3 maxVec = frustumCornersWS[0];
	for (int i = 1; i < 8; ++i) {
		minVec = nex::minVec(minVec, frustumCornersWS[i]);
		maxVec = nex::maxVec(maxVec, frustumCornersWS[i]);
	}

	frustumCenter = minVec + 0.5f * (maxVec - minVec)/2.0f;*/


	// calc sphere that tightly encloses the frustum
	// We use the max distance from the frustum center to the corners
	// TODO Actually should the distance to all corners be the same???
	float radius = 0.0f;
	for (unsigned int i = 0; i < 8; ++i)
	{
		float distance = glm::length(frustumCornersWS[i] - frustumCenter);
		radius = std::max<float>(radius, distance);
	}

	// do some rounding for fighting numerical issues
	// This helps to reduce flickering
	// Note that we use here a different formula than in function calcSplitDistances, as it produces better results 
	radius = std::round(radius *16.0f) / 16.0f;

	return { frustumCenter, radius };
}

bool CascadedShadow::cascadeNeedsUpdate(const glm::mat4& shadowView, int cascadeIdx, const glm::vec3& newCenter,
	const glm::vec3& oldCenter, float cascadeBoundRadius, glm::vec3* offset)
{
	// Find the offset between the new and old bound center
	const glm::vec3 oldCenterInCascade = glm::vec3(shadowView * glm::vec4(oldCenter, 1.0f));
	const glm::vec3 newCenterInCascade = glm::vec3(shadowView * glm::vec4(newCenter, 1.0f));
	const glm::vec3 centerDiff = newCenterInCascade - oldCenterInCascade;

	// Find the pixel size based on the diameters and map pixel size
	const float pixelSize = (float)mShadowMapSize / (2.0f * cascadeBoundRadius);

	const float pixelOffX = centerDiff.x * pixelSize;
	const float pixelOffY = centerDiff.y * pixelSize;
	//const float pixelOffZ = centerDiff.z * pixelSize;

	// Check if the center moved at least half a pixel unit
	const bool needUpdate = abs(pixelOffX) > 0.5f || abs(pixelOffY) > 0.5f; //|| abs(pixelOffZ) > 0.5f;
	if (needUpdate)
	{
		// Round to the 
		offset->x = floorf(0.5f + pixelOffX) / pixelSize;
		offset->y = floorf(0.5f + pixelOffY) / pixelSize;
		//offset->z = floorf(0.5f + pixelOffZ) / pixelSize;
		offset->z = centerDiff.z;
	}

	return needUpdate;
}

CascadedShadow::DepthPass::DepthPass(unsigned numCascades, bool useBones) : mNumCascades(numCascades)
{
	std::vector<std::string> defines;
	if (useBones) {
		defines.push_back("#define BONE_ANIMATION 1");
	}

	mProgram = ShaderProgram::create("shadow/cascaded_depth_vs.glsl", "shadow/cascaded_depth_fs.glsl", nullptr, nullptr, nullptr, defines);
}

void nex::CascadedShadow::DepthPass::setCascadeIndexRaw(unsigned index)
{
	mCascadeIndex = index;
}

void CascadedShadow::DepthPass::setCascadeIndex(unsigned index)
{
	static const UniformLocation CASCADE_INDEX_LOCATION = 0;
	mProgram->setUInt(CASCADE_INDEX_LOCATION, index);
}

void CascadedShadow::DepthPass::updateConstants(const RenderContext& constants)
{
	const auto& camera = *constants.camera;
	TransformShader::updateConstants(constants);
	setCascadeIndex(mCascadeIndex);
}

CascadedShadow::CascadeDataPass::CascadeDataPass(unsigned numCascades) : ComputeShader(), mNumCascades(numCascades)
{
	std::vector<std::string> defines{ std::string("#define CSM_NUM_CASCADES ") + std::to_string(mNumCascades) };
	mProgram = ShaderProgram::createComputeShader("SDSM/cascade_data_cs.glsl", defines);
	bind();
	//CascadeData::calcCascadeDataByteSize(numCascades)
	mSharedOutput = std::make_unique<ShaderStorageBuffer>(0, sizeof(CascadeData), nullptr, ShaderBuffer::UsageHint::STREAM_DRAW);

	mPrivateOutput = std::make_unique<ShaderStorageBuffer>(1, sizeof(glm::vec4) * numCascades, nullptr, ShaderBuffer::UsageHint::STREAM_READ);
	mInputBuffer = std::make_unique<ShaderStorageBuffer>(2, sizeof(Input), nullptr, ShaderBuffer::UsageHint::STREAM_DRAW);
	//mDistanceInputBuffer = std::make_unique<ShaderStorageBuffer>(1, sizeof(DistanceInput), ShaderBuffer::UsageHint::DYNAMIC_DRAW);
	
	mUseAntiFlickering = { mProgram->getUniformLocation("useAntiFlickering"), UniformType::UINT};

	resetPrivateData();
}

ShaderStorageBuffer* CascadedShadow::CascadeDataPass::getSharedOutput()
{
	return mSharedOutput.get();
}

void CascadedShadow::CascadeDataPass::useDistanceInputBuffer(ShaderStorageBuffer* buffer)
{
	buffer->bindToTarget(3);
	//buffer->syncWithGPU();
}

void CascadedShadow::CascadeDataPass::setUseAntiFlickering(bool use)
{
	mProgram->setUInt(mUseAntiFlickering.location, use);
}

void CascadedShadow::CascadeDataPass::update(const Input& input)
{
	mInputBuffer->bindToTarget();
	mInputBuffer->update(sizeof(Input), &input, 0);
}

void CascadedShadow::CascadeDataPass::resetPrivateData()
{
	mPrivateOutput->bindToTarget();

	std::vector<glm::vec4> data(mNumCascades);

	for (auto i = 0; i < data.size(); ++i)
	{
		data[i] = glm::vec4(0.0f);
	}

	mPrivateOutput->update(data.size() * sizeof(glm::vec4), data.data(), 0);
	mPrivateOutput->map(ShaderBuffer::Access::READ_ONLY);
	mPrivateOutput->unmap();
}


void CascadedShadow::frameUpdate(const Camera& camera, const glm::vec3& lightDirection, Texture2D* depth)
{
	const auto nearDistance = camera.getNearDistance();
	const auto farDistance = camera.getFarDistance();

	if (mUseTightNearFarPlane && depth != nullptr)
	{
		mSceneNearFarComputeShader->bind();

		unsigned xDim = 16 * 16; // 256
		unsigned yDim = 8 * 8; // 128

		const auto width = depth->getWidth();
		const auto height = depth->getHeight();

		unsigned dispatchX = width % xDim == 0 ? width / xDim : width / xDim + 1;
		unsigned dispatchY = height % yDim == 0 ? height / yDim : height / yDim + 1;

		const auto frustum = camera.getFrustum();

		mSceneNearFarComputeShader->setConstants(nearDistance + 0.05, farDistance - 0.05, camera.getProjectionMatrix());
		mSceneNearFarComputeShader->setDepthTexture(depth);

		mSceneNearFarComputeShader->dispatch(dispatchX, dispatchY, 1);

		frameUpdateTightNearFarPlane(camera, lightDirection, mSceneNearFarComputeShader->getWriteOutBuffer());
	} else
	{
		frameUpdateNoTightNearFarPlane(camera, lightDirection, glm::vec2(nearDistance, farDistance));
	}
}

bool CascadedShadow::getAntiFlickering() const
{
	return mAntiFlickerOn;
}

bool nex::CascadedShadow::getUseLogarithmicSplits() const
{
	return mUseLogarithmicSplits;
}

float CascadedShadow::getBiasMultiplier() const
{
	return mBiasMultiplier;
}

void CascadedShadow::setBiasMultiplier(float bias, bool informObservers)
{
	mBiasMultiplier = bias;
	if (informObservers) informCascadeChanges();
}

TransformShader* CascadedShadow::getDepthPass()
{
	return mDepthPass.get();
}

unsigned CascadedShadow::getHeight() const
{
	return mCascadeHeight;
}

const PCFFilter& CascadedShadow::getPCF() const
{
	return mPCF;
}

float CascadedShadow::getShadowStrength() const
{
	return mShadowStrength;
}

bool nex::CascadedShadow::getVisualizeCascades() const
{
	return mVisualizeCascades;
}

void CascadedShadow::setShadowStrength(float strength)
{
	mShadowStrength = strength;
}

ShaderStorageBuffer* CascadedShadow::getCascadeBuffer()
{
	return mDataComputePass->getSharedOutput();
}

const ShaderStorageBuffer* CascadedShadow::getCascadeBuffer() const
{
	return mDataComputePass->getSharedOutput();
}


void CascadedShadow::useTightNearFarPlane(bool use)
{
	mUseTightNearFarPlane = use;
}

void nex::CascadedShadow::useLogarithmicSplits(bool use)
{
	mUseLogarithmicSplits = use;
}

unsigned CascadedShadow::getWidth() const
{
	return mCascadeWidth;
}

const glm::mat4& CascadedShadow::getWorldToShadowSpace() const
{
	return mGlobal.worldToShadowSpace;
}

const glm::mat4& CascadedShadow::getShadowView() const
{
	return mGlobal.shadowView;
}

void CascadedShadow::frameReset()
{
	if (mUseTightNearFarPlane)
	{
		mSceneNearFarComputeShader->reset();
	}
}

void nex::CascadedShadow::render(const nex::RenderCommandQueue::Buffer& shadowCommands, const nex::RenderContext& constants)
{
	bind(constants);

	ShaderOverride<nex::Shader> overrides;
	overrides.default = mDepthPass.get();
	overrides.rigged = mDepthPassBones.get();

	const auto& state = RenderState::getDefault();

	for (unsigned i = 0; i < mNumCascades; ++i)
	{
		begin(i);
		
		Drawer::draw(shadowCommands, constants, overrides, &state);
		/*for (const auto& command : shadowCommands)
		{
			mDepthPass->setModelMatrix(*command.worldTrafo, *command.prevWorldTrafo);
			mDepthPass->uploadTransformMatrices();

			for (const auto& pair : command.batch->getEntries()) {
				Drawer::draw(mDepthPass.get(), pair.first, nullptr);
			}
			
		}*/
	}
}

void CascadedShadow::setAntiFlickering(bool enable)
{
	mAntiFlickerOn = enable;
}

void CascadedShadow::setPCF(const PCFFilter& filter, bool informOberservers)
{
	mPCF = filter;
	if (informOberservers) informCascadeChanges();
}

void nex::CascadedShadow::setVisualizeCascades(bool visualize, bool informOberservers)
{
	mVisualizeCascades = visualize;
	if (informOberservers) informCascadeChanges();
}

void CascadedShadow::resizeCascadeData(unsigned numCascades, bool informObservers)
{
	mNumCascades = numCascades;
	mSplitDistances.resize(numCascades);
	mCascadeBoundCenters.resize(numCascades);

	// reset cascade data 
	for (int i = 0; i < numCascades; i++)
	{
		mCascadeBoundCenters[i] = glm::vec3(0.0f);
		mSplitDistances[i] = 0.0f;
		mCascadeData.cascadedSplits[i] = glm::vec4(0.0f);
		mCascadeData.scaleFactors[i] = glm::vec4(0.0f);
		mCascadeData.lightViewProjectionMatrices[i] = glm::mat4(0.0f);
	}


	updateTextureArray();

	mDataComputePass = std::make_unique<CascadeDataPass>(numCascades);
	mDepthPass = std::make_unique<DepthPass>(numCascades, false);
	mDepthPassBones = std::make_unique<DepthPass>(numCascades, true);

	if (informObservers)
	{
		informCascadeChanges();
	}
}

const nex::CascadeData& CascadedShadow::getCascadeData() const
{
	return mCascadeData;
}

unsigned nex::CascadedShadow::getNumCascades() const
{
	return mNumCascades;
}

CascadedShadow_ConfigurationView::CascadedShadow_ConfigurationView(CascadedShadow* model) : mModel(model),
mCascadeView({}, { 256, 256 })
{

	auto numConfigApply = [this]() {
		mModel->resizeCascadeData(mNumCascades, true);
	};

	auto numConfigRevert = [this]() {
		mNumCascades = mModel->getNumCascades();
	};

	auto numConfigCond = [this]() {
		return mNumCascades != mModel->getNumCascades();
	};

	mNumConfigApplyButton = std::make_unique<nex::gui::ApplyButton>(numConfigApply, numConfigRevert, numConfigCond);

	auto biasApply = [this]() {
		mModel->setBiasMultiplier(mBias, true);
	};
	auto biasRevert = [this]() {
		mBias = mModel->getBiasMultiplier();
	};

	auto biasCond = [this]() {
		return mBias != mModel->getBiasMultiplier();
	};

	mBiasApplyButton = std::make_unique<nex::gui::ApplyButton>(biasApply, biasRevert, biasCond);


	auto cascadeDimensionApply = [this]() {
		mModel->resize(mCascadeDimension.x, mCascadeDimension.y);
	};
	auto cascadeDimensionRevert = [this]() {
		mCascadeDimension = glm::uvec2(mModel->getWidth(), mModel->getHeight());
	};

	auto cascadeDimensionCond = [this]() {
		return mCascadeDimension != glm::uvec2(mModel->getWidth(), mModel->getHeight());
	};

	mCascadeDimensioApplyButton = std::make_unique<nex::gui::ApplyButton>(cascadeDimensionApply, cascadeDimensionRevert, cascadeDimensionCond);

	auto pcfApply = [this]() {
		mModel->setPCF(mPcf);
	};
	auto pcfRevert = [this]() {
		mPcf = mModel->getPCF();
	};

	auto pcfCond = [this]() {
		return !(mPcf == mModel->getPCF());
	};

	mPcfApplyButton = std::make_unique<nex::gui::ApplyButton>(pcfApply, pcfRevert, pcfCond);


	auto visualizeApply = [this]() {
		mModel->setVisualizeCascades(mVisualizeCascades);
	};
	auto visualizeRevert = [this]() {
		mVisualizeCascades = mModel->getVisualizeCascades();
	};
	auto visualizeCond = [this]() {
		return !(mVisualizeCascades == mModel->getVisualizeCascades());
	};

	mVisualizeCascadesApplyButton = std::make_unique<nex::gui::ApplyButton>(visualizeApply, visualizeRevert, visualizeCond);


	// apply default state
	numConfigRevert();
	biasRevert();
	cascadeDimensionRevert();
	pcfRevert();
	visualizeRevert();
}

void CascadedShadow_ConfigurationView::drawShadowStrengthConfig()
{
	float strength = mModel->getShadowStrength();
	if (ImGui::SliderFloat("Shadow Strength", &strength, 0.0f, 1.0f))
	{
		mModel->setShadowStrength(strength);
	}
}

void CascadedShadow_ConfigurationView::drawCascadeNumConfig()
{
	const unsigned realNumber(mModel->getNumCascades());

	ImGuiContext& g = *GImGui;
	ImGui::BeginGroup();

	unsigned minVal = 1;
	unsigned maxVal = CSM_MAX_NUM_CASCADES;

	ImGui::DragScalar("Number of cascades", ImGuiDataType_U32, &mNumCascades, 0.0f, &minVal, &maxVal);
	//ImGui::InputScalar("Number of cascades", ImGuiDataType_U32, &mNumCascades);
	//mNumCascades = std::clamp<unsigned>(mNumCascades, 1, CSM_MAX_NUM_CASCADES);


	mNumConfigApplyButton->drawGUI();

	ImGui::EndGroup();
}

void CascadedShadow_ConfigurationView::drawCascadeBiasConfig()
{
	ImGuiContext& g = *GImGui;
	ImGui::BeginGroup();
	ImGui::InputScalar("Bias multiplier", ImGuiDataType_Float, &mBias);
	
	mBiasApplyButton->drawGUI();

	ImGui::EndGroup();
}

void CascadedShadow_ConfigurationView::drawCascadeDimensionConfig()
{
	ImGuiContext& g = *GImGui;
	ImGui::BeginGroup();
	ImGui::InputScalarN("Cascade Dimension", ImGuiDataType_U32, &mCascadeDimension, 2);

	mCascadeDimensioApplyButton->drawGUI();

	ImGui::EndGroup();
}

void CascadedShadow_ConfigurationView::drawPCFConfig()
{

	ImGuiContext& g = *GImGui;
	ImGui::BeginGroup();
	ImGui::InputScalarN("PCF Samples", ImGuiDataType_U32, &mPcf.sampleCountX, 2);
	ImGui::Checkbox("PCF Lerp filtering", &mPcf.useLerpFiltering);

	mPcfApplyButton->drawGUI();

	ImGui::EndGroup();
}

void nex::CascadedShadow_ConfigurationView::drawVisualizeCascadesConfig()
{
	ImGuiContext& g = *GImGui;
	ImGui::BeginGroup();
	ImGui::Checkbox("Visualize Cascades", &mVisualizeCascades);
	mVisualizeCascadesApplyButton->drawGUI();
	ImGui::EndGroup();
}

void CascadedShadow_ConfigurationView::drawSelf()
{
	ImGui::PushID(mId.c_str());
	//m_pbr
	ImGui::LabelText("", "CSM:");
	ImGui::Dummy(ImVec2(0, 20));

	bool isEnabled = mModel->isEnabled();

	if (ImGui::Checkbox("Enable CSM", &isEnabled))
	{
		RenderEngine::getCommandQueue()->push([=]() {
			mModel->enable(isEnabled, true);
		});
	}

	//drawShadowStrengthConfig();

	bool useLogarithmicSplits = mModel->getUseLogarithmicSplits();
	if (ImGui::Checkbox("Logarithmic splits", &useLogarithmicSplits))
	{
		mModel->useLogarithmicSplits(useLogarithmicSplits);
	}


	bool enableAntiFlickering = mModel->getAntiFlickering();

	if (ImGui::Checkbox("Anti Flickering", &enableAntiFlickering))
	{
		mModel->setAntiFlickering(enableAntiFlickering);
	}

	drawCascadeNumConfig();

	drawCascadeDimensionConfig();

	drawCascadeBiasConfig();

	drawPCFConfig();

	drawVisualizeCascadesConfig();

	if (ImGui::TreeNode("Cascades"))
	{
		auto* texture = mModel->getDepthTextureArray();
		auto& imageDesc = mCascadeView.getTextureDesc();
		imageDesc.texture = texture;
		imageDesc.flipY = ImageFactory::isYFlipped();
		imageDesc.sampler = nullptr;
		mCascadeView.drawGUI();

		ImGui::TreePop();
	}

	nex::gui::Separator(2.0f);
	
	ImGui::PopID();
}