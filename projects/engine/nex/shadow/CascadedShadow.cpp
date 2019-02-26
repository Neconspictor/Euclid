#include <nex/shadow/CascadedShadow.hpp>
#include <nex/mesh/SubMesh.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/RenderBackend.hpp>
#include <glm/gtc/matrix_transform.inl>
#include "nex/FileSystem.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
using namespace nex;

CascadedShadow::CascadedShadow(unsigned int cascadeWidth, unsigned int cascadeHeight, bool antiFlickerOn) :
	mCascadeWidth(cascadeWidth),
	mCascadeHeight(cascadeHeight),
	mShadowMapSize(std::max<int>(cascadeWidth, cascadeHeight)),
	mAntiFlickerOn(antiFlickerOn)
{
	// reset cascade cound centers 
	for (int i = 0; i < NUM_CASCADES; i++)
	{
		mCascadeBoundCenters[i] = glm::vec3(0.0f);
	}

	updateTextureArray();
}

void CascadedShadow::begin(int cascadeIndex)
{
	mDepthPassShader.bind();

	mRenderTarget.bind();
	RenderBackend::get()->setViewPort(0, 0, mCascadeWidth, mCascadeHeight);
	RenderBackend::get()->setScissor(0, 0, mCascadeWidth, mCascadeHeight);

	auto* depth = mRenderTarget.getDepthAttachment();
	depth->layer = cascadeIndex;
	mRenderTarget.updateDepthAttachment();

	//TODO validate!
	//glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *((TextureGL*)mDepthTextureArray->getImpl())->getTexture(), 0, cascadeIndex);

	mRenderTarget.clear(RenderComponent::Depth);
	RenderBackend::get()->getDepthBuffer()->enableDepthTest(true);

	// We use depth clamping so that the shadow maps keep from moving through objects which causes shadows to disappear.
	RenderBackend::get()->getDepthBuffer()->enableDepthClamp(true);
	RenderBackend::get()->getRasterizer()->setCullMode(PolygonSide::BACK);

	glm::mat4 lightViewProjection = mCascadeData.lightViewProjectionMatrices[cascadeIndex];

	// update lightViewProjectionMatrix uniform
	static const UniformLocation LIGHT_VIEW_PROJECTION_MATRIX_LOCATION = 0;
	mDepthPassShader.getProgram()->setMat4(LIGHT_VIEW_PROJECTION_MATRIX_LOCATION, lightViewProjection);
	//glUniformMatrix4fv(LIGHT_VIEW_PROJECTION_MATRIX_LOCATION, 1, GL_FALSE, &lightViewProjection[0][0]);
}

void CascadedShadow::end()
{
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	mDepthPassShader.unbind();
	//###glDisable(GL_DEPTH_TEST);
	// disable depth clamping
	RenderBackend::get()->getDepthBuffer()->enableDepthClamp(false);
	RenderBackend::get()->getRasterizer()->setCullMode(PolygonSide::BACK);
}

Texture* CascadedShadow::getDepthTextureArray()
{
	return mRenderTarget.getDepthAttachment()->texture.get();
}

void CascadedShadow::resize(unsigned cascadeWidth, unsigned cascadeHeight)
{
	mCascadeWidth = cascadeWidth;
	mCascadeHeight = cascadeHeight;
	mShadowMapSize = std::max<int>(cascadeWidth, cascadeHeight);

	// reset cascade cound centers 
	for (int i = 0; i < NUM_CASCADES; i++)
	{
		mCascadeBoundCenters[i] = glm::vec3(0.0f);
	}

	updateTextureArray();
}

/*void CascadedShadowGL::render(SubMesh* mesh, const glm::mat4* modelMatrix)
{
	// Update modelMatrix uniform
	static const GLuint MODEL_MATRIX_LOCATION = 1;
	glUniformMatrix4fv(MODEL_MATRIX_LOCATION, 1, GL_FALSE, &(*modelMatrix)[0][0]);

	// render mesh
	const VertexArray* vertexArray = mesh->getVertexArray();
	const IndexBuffer* indexBuffer = mesh->getIndexBuffer();

	vertexArray->bind();
	indexBuffer->bind();
	glDrawElements(GL_TRIANGLES, indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr);

	indexBuffer->unbind();
	vertexArray->unbind();
}*/

void CascadedShadow::updateTextureArray()
{
	TextureData data;
	data.colorspace = ColorSpace::DEPTH;
	data.internalFormat = InternFormat::DEPTH_COMPONENT32F;
	data.pixelDataType = PixelDataType::FLOAT;
	data.minFilter = TextureFilter::NearestNeighbor; // IMPORTANT: Linear filter produces ugly artifacts when using PCF filtering
	data.magFilter = TextureFilter::NearestNeighbor; // IMPORTANT: Linear filter produces ugly artifacts when using PCF filtering
	data.wrapR = data.wrapS = data.wrapT = TextureUVTechnique::ClampToEdge;
	data.useDepthComparison = true;
	data.compareFunc = CompareFunction::LESS;

	RenderAttachment depth;
	depth.type = RenderAttachment::Type::DEPTH;
	depth.target = TextureTarget::TEXTURE2D_ARRAY;
	depth.texture = std::make_unique<Texture2DArray>(mCascadeWidth, mCascadeHeight, NUM_CASCADES, false, data, nullptr);

	mRenderTarget.bind();
	mRenderTarget.useDepthAttachment(std::move(depth));
	mRenderTarget.finalizeAttachments();
	mRenderTarget.enableDrawToColorAttachments(false);
	mRenderTarget.enableReadFromColorAttachments(false);
	mRenderTarget.assertCompletion();
}

void CascadedShadow::frameUpdateNew(Camera* camera, const glm::vec3& lightDirection)
{
	const float minDistance = 0.0f;
	//const Frustum& frustum = camera->getFrustum(ProjectionMode::Perspective);
	//const float cameraNearPlaneVS = frustum.nearPlane;
	calcSplitSchemes(camera);
	mCascadeData.inverseViewMatrix = inverse(camera->getView());
	const auto global = calcShadowSpaceMatrix(camera, lightDirection);
	const glm::mat3 shadowOffsetMatrix = glm::mat3(glm::transpose(global.shadowView));
	static const float orthoZMin = 10.0f;


	for (unsigned int cascadeIterator = 0; cascadeIterator < NUM_CASCADES; ++cascadeIterator)
	{
		// the far plane of the previous cascade is the near plane of the current cascade
		//const float nearPlane = cascadeIterator == 0 ? cameraNearPlaneVS : mCascadeData.cascadedFarPlanes[cascadeIterator - 1].x;
		//const float farPlane = mCascadeData.cascadedFarPlanes[cascadeIterator].x;
		const float nearSplitDistance = cascadeIterator == 0 ? minDistance : mSplitDistances[cascadeIterator - 1];
		const float farSplitDistance = mSplitDistances[cascadeIterator];

		const auto boundingSphere = extractFrustumBoundSphere(camera, nearSplitDistance, farSplitDistance);
		const float radius = boundingSphere.radius;
		const glm::vec3& frustumCenterWS = boundingSphere.center;

		//Position the view matrix looking down the center of the frustum with the light direction
		// We multiply the normalized light direction by radius so that nothing of the shadow map gets clipped
		glm::vec3 lightPositionWS = frustumCenterWS + normalize(lightDirection);

		glm::vec3 up (0.0f, 1.0f, 0.0f);

		if (length(cross(up, camera->getLook())) < 0.1f)
		{
			up = glm::vec3(1.0f, 0.0f, 0.0f);
		}

		glm::mat4 cascadeViewMatrix = glm::lookAt(frustumCenterWS, lightPositionWS, up);



		glm::vec3 maxExtents;
		glm::vec3 minExtents;
		glm::mat4 cascadeProjMatrix;

		if (mAntiFlickerOn)
		{
			// To avoid anti flickering we need to make the transformation invariant to camera rotation and translation
			// By encapsulating the cascade frustum with a sphere we achive the rotation invariance
			maxExtents = glm::vec3(radius, radius, radius);
			minExtents = -maxExtents;
			//glm::vec3 cascadeExtents = maxExtents - minExtents;
		} else
		{
			glm::vec3 frustumPointsWS[8];
			extractFrustumPoints(camera, nearSplitDistance, farSplitDistance, frustumPointsWS);
			maxExtents = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
			minExtents = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);

			for (auto i = 0; i < 8; ++i)
			{
				glm::vec3 pointInShadowSpace = cascadeViewMatrix * glm::vec4(frustumPointsWS[i], 1.0f);
				minExtents = minVec(minExtents, pointInShadowSpace);
				maxExtents = maxVec(maxExtents, pointInShadowSpace);
			}
		}

		// Calculate the projection matrix by defining the AABB of the cascade (as ortho projection)
		float zComponent = max(radius, 10.0f);
		cascadeProjMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, -zComponent, zComponent);

		

		if (mAntiFlickerOn)
		{
			// Calculate the rounding matrix that ensures that shadow edges do not shimmer
			// At first we need the shadow space transformation matrix
			glm::mat4 shadowMatrix = cascadeProjMatrix * cascadeViewMatrix;

			//glm::vec4 shadowOrigin = glm::vec4(100.0f, -333.0f, 0.0f, 1.0f); // The origin in world space; Actually it is irrelevant which reference point we use.
																		// Just any point in world space
			const glm::vec4 shadowOriginWS = shadowMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Get the shadow space coordinates of the sample point shadowOrigin;//
			
			
			glm::vec4 shadowOriginTranslated = shadowOriginWS * mShadowMapSize / 2.0f; // Todo: Why multiply by  mShadowMapSize / 2.0f ?

			glm::vec4 roundedOrigin = glm::round(shadowOriginTranslated);
			glm::vec4 roundOffset = roundedOrigin - shadowOriginTranslated;
			roundOffset = roundOffset * 2.0f / mShadowMapSize; //  2.0f / mShadowMapSize probably reverses previous mShadowMapSize / 2.0f mulitplication
			//roundOffset.z = 0.0f;
			roundOffset.w = 0.0f;

			cascadeProjMatrix[3] += roundOffset; // adjust translation in (x,y) plane
		}


		mCascadeData.lightViewProjectionMatrices[cascadeIterator] = cascadeProjMatrix * cascadeViewMatrix;
	}
}

void CascadedShadow::frameUpdateOld(Camera* camera, const glm::vec3& lightDirection)
{
	mCascadeData.inverseViewMatrix = inverse(camera->getView());

	Frustum frustum = camera->getFrustum(ProjectionMode::Perspective);

	//Start off by calculating the split distances
	float cascadeSplits[NUM_CASCADES] = { 0 };

	//Between 0 and 1, change in order to see the results
	float lambda = 1.0f;
	//Between 0 and 1, change these to check the results
	float minDistance = 0.0f;
	float maxDistance = 1.0f;


	float nearClip = frustum.nearPlane;
	float farClip = frustum.farPlane;
	float clipRange = farClip - nearClip;

	float minZ = nearClip + minDistance * clipRange;
	float maxZ = nearClip + maxDistance * clipRange;

	float range = maxZ - minZ;
	float ratio = maxZ / minZ;

	// We calculate the splitting planes of the view frustum by using an algorithm 
	// created by NVIDIA: The algorithm works by using a logarithmic and uniform split scheme.
	for (unsigned int i = 0; i < NUM_CASCADES; ++i)
	{
		float p = (i + 1) / static_cast<float>(NUM_CASCADES);
		float log = minZ * std::pow(ratio, p);
		float uniform = minZ + range * p;
		float d = lambda * (log - uniform) + uniform;
		cascadeSplits[i] = (d - nearClip) / clipRange;
	}

	for (unsigned int cascadeIterator = 0; cascadeIterator < NUM_CASCADES; ++cascadeIterator)
	{

		float prevSplitDistance = cascadeIterator == 0 ? minDistance : cascadeSplits[cascadeIterator - 1];
		float splitDistance = cascadeSplits[cascadeIterator];

		// At first we define the frustum corners in NDC space
		glm::vec3 frustumCornersWS[8] =
		{
			glm::vec3(-1.0f,  1.0f, -1.0f),
			glm::vec3(1.0f,  1.0f, -1.0f),
			glm::vec3(1.0f, -1.0f, -1.0f),
			glm::vec3(-1.0f, -1.0f, -1.0f),
			glm::vec3(-1.0f,  1.0f,  1.0f),
			glm::vec3(1.0f,  1.0f,  1.0f),
			glm::vec3(1.0f, -1.0f,  1.0f),
			glm::vec3(-1.0f, -1.0f,  1.0f),
		};

		// Now we transform the frustum corners back to world space
		glm::mat4 invViewProj = glm::inverse(camera->getPerspProjection() * camera->getView());
		for (unsigned int i = 0; i < 8; ++i)
		{
			glm::vec4 inversePoint = invViewProj * glm::vec4(frustumCornersWS[i], 1.0f);
			frustumCornersWS[i] = glm::vec3(inversePoint / inversePoint.w);
		}

		// Calculate rays that define the near and far plane of each cascade split.
		// Than we translate the frustum corner accordingly so that the frustum starts at the near splitting plane
		// and ends at the far splitting plane.
		for (unsigned int i = 0; i < 4; ++i)
		{
			glm::vec3 cornerRay = frustumCornersWS[i + 4] - frustumCornersWS[i];
			glm::vec3 nearCornerRay = cornerRay * prevSplitDistance;
			glm::vec3 farCornerRay = cornerRay * splitDistance;
			frustumCornersWS[i + 4] = frustumCornersWS[i] + farCornerRay;
			frustumCornersWS[i] = frustumCornersWS[i] + nearCornerRay;
		}

		// calc center of the frustum
		glm::vec3 frustumCenter = glm::vec3(0.0f);
		for (unsigned int i = 0; i < 8; ++i)
			frustumCenter += frustumCornersWS[i];
		frustumCenter /= 8.0f;

		//float far = -INFINITY;
		//float near = INFINITY;

		static bool debug = false;

		if (debug)
		{
			std::ofstream file("frustumCorners-old.txt");
			for (auto i = 0; i < 8; ++i)
			{
				file << "Corner " << i << " = " << glm::to_string<glm::vec3>(frustumCornersWS[i]) << std::endl;
			}

			file << "center = " << glm::to_string<glm::vec3>(frustumCenter) << std::endl;
		}

		// calc sphere that tightly encloses the frustum
		// We use the max distance from the frustum center to the corners
		// TODO Actually should the distance ot all corners be the same???
		float radius = 0.0f;
		for (unsigned int i = 0; i < 8; ++i)
		{
			float distance = glm::length(frustumCornersWS[i] - frustumCenter);
			radius = std::max<float>(radius, distance);
		}
		radius = std::ceil(radius * 16.0f) / 16.0f; // alignment???


		glm::vec3 maxExtents = glm::vec3(radius, radius, radius);
		glm::vec3 minExtents = -maxExtents;
		glm::vec3 cascadeExtents = maxExtents - minExtents;


		//Position the view matrix looking down the center of the frustum with the light direction
		// We multiply the normalized light direction by radius so that nothing of the shadow map gets clipped
		glm::vec3 lightPositionWS = frustumCenter - normalize(lightDirection) * radius;
		glm::mat4 mLightViewMatrix = glm::lookAt(lightPositionWS, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));

		// Calculate the projection matrix by defining the AABB of the cascade (as ortho projection)
		glm::mat4 mLightProjMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, cascadeExtents.z);

		// Calculate the rounding matrix that ensures that shadow edges do not shimmer
		// At first we need the shadow space transformation matrix
		glm::mat4 shadowMatrix = mLightProjMatrix * mLightViewMatrix;

		glm::vec4 shadowOrigin = glm::vec4(100.0f, -333.0f, 0.0f, 1.0f); // The origin in world space; Actually it is irrelevant which reference point we use.
																	// Just any point in world space
		shadowOrigin = shadowMatrix * shadowOrigin; // Get the shadow space coordinates of the sample point
		shadowOrigin = shadowOrigin * mShadowMapSize / 2.0f; // Todo: Why multiply by  mShadowMapSize / 2.0f ?

		glm::vec4 roundedOrigin = glm::round(shadowOrigin);
		glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
		roundOffset = roundOffset * 2.0f / mShadowMapSize; //  2.0f / mShadowMapSize probably reverses previous mShadowMapSize / 2.0f mulitplication
		roundOffset.z = 0.0f;
		roundOffset.w = 0.0f;

		glm::mat4 shadowProj = mLightProjMatrix;
		shadowProj[3] += roundOffset; // adjust translation in (x,y) plane
		mLightProjMatrix = shadowProj;

		//Store the split distances and the relevant matrices
		const float clipDist = clipRange;
		mCascadeData.cascadedFarPlanes[cascadeIterator].x = (nearClip + splitDistance * clipDist);
		mCascadeData.lightViewProjectionMatrices[cascadeIterator] = mLightProjMatrix * mLightViewMatrix;
	}
}

CascadedShadow::GlobalShadow CascadedShadow::calcShadowSpaceMatrix(Camera* camera, const glm::vec3& lightDirection)
{
	const Frustum& frustum = camera->getFrustum(ProjectionMode::Perspective);
	const float cascadeTotalRange = frustum.farPlane - frustum.nearPlane;
	const auto shadowBounds = extractFrustumBoundSphere(camera, frustum.nearPlane, frustum.farPlane);

	// Find the view matrix
	const glm::vec3 cameraFrustumCenterWS = camera->getPosition() + camera->getLook() * cascadeTotalRange * 0.5f;
	const glm::vec3 lookAt = cameraFrustumCenterWS + normalize(lightDirection) * frustum.farPlane;

	const glm::vec3 lightPos = cameraFrustumCenterWS - normalize(lightDirection) * shadowBounds.radius;

	glm::vec3 upVec = glm::normalize(glm::cross(lightDirection, glm::vec3(1.0f, 0.0f, 0.0f)));
	if (glm::length(upVec) < 0.999f)
	{
		upVec = glm::normalize(glm::cross(lightDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
	}

	const glm::mat4 shadowView = glm::lookAt(lightPos, cameraFrustumCenterWS, glm::vec3(0,1,0));


	// Get the bounds for the shadow space
	
	const auto shadowProj = glm::ortho(-shadowBounds.radius, shadowBounds.radius,
		-shadowBounds.radius, shadowBounds.radius,
		0.0f, shadowBounds.radius*2);

	// The combined transformation from world to shadow space
	GlobalShadow result;
	result.worldToShadowSpace = shadowProj * shadowView;
	result.shadowView = shadowView;
	result.radius = shadowBounds.radius;
	return result;
}

void CascadedShadow::calcSplitSchemes(Camera* camera)
{
	calcSplitDistances(camera);

	Frustum frustum = camera->getFrustum(ProjectionMode::Perspective);
	const float nearClip = frustum.nearPlane;
	const float farClip = frustum.farPlane;
	const float clipRange = farClip - nearClip;

	// We calculate the splitting planes of the view frustum by using an algorithm 
	// created by NVIDIA: The algorithm works by using a logarithmic and uniform split scheme.
	for (unsigned int i = 0; i < NUM_CASCADES; ++i)
	{
		mCascadeData.cascadedFarPlanes[i].x = (nearClip + mSplitDistances[i] * clipRange);
	}
}

void CascadedShadow::calcSplitDistances(Camera* camera)
{
	Frustum frustum = camera->getFrustum(ProjectionMode::Perspective);
	//Between 0 and 1, change in order to see the results
	const float lambda = 1.0f;
	//Between 0 and 1, change these to check the results
	const float minDistance = 0.0f;
	const float maxDistance = 1.0f;


	const float nearClip = frustum.nearPlane;
	const float farClip = frustum.farPlane;
	const float clipRange = farClip - nearClip;

	const float minZ = nearClip + minDistance * clipRange;
	const float maxZ = nearClip + maxDistance * clipRange;

	const float range = maxZ - minZ;
	const float ratio = maxZ / minZ;

	// We calculate the splitting planes of the view frustum by using an algorithm 
	// created by NVIDIA: The algorithm works by using a logarithmic and uniform split scheme.
	for (unsigned int i = 0; i < NUM_CASCADES; ++i)
	{
		const float p = (i + 1) / static_cast<float>(NUM_CASCADES);
		const float log = minZ * std::pow(ratio, p);
		const float uniform = minZ + range * p;
		const float d = lambda * (log - uniform) + uniform;
		mSplitDistances[i] = (d - nearClip) / clipRange;
	}
}

CascadedShadow::BoundingSphere CascadedShadow::extractFrustumBoundSphere(Camera* camera, float nearSplitDistance, float farSplitDistance)
{
	/*const auto& position = camera->getPosition();
	const auto& up = camera->getUp();
	const auto& look = camera->getLook();
	const auto& right = camera->getRight();


	const float aspectRatio = camera->getAspectRatio();
	const float fov = glm::radians(camera->getFOV());

	// Calculate the tangent values (this can be cached as long as the FOV doesn't change)
	const float tanFOVX = tanf(aspectRatio * fov / 2.0f);
	const float tanFOVY = tanf(fov / 2.0f);

	// The center of the sphere is in the center of the frustum
	const auto boundCenter = position + look * (nearPlane + 0.5f * (nearPlane + farPlane));

	// Radius is the distance to one of the frustum far corners
	// TODO use right instead of -right ???
	const auto boundSpan = position + (-right * tanFOVX + up * tanFOVY + look) * farPlane - boundCenter;
	const auto boundRadius = glm::length(boundSpan);*/



	glm::vec3 frustumCornersWS[8];
	extractFrustumPoints(camera, nearSplitDistance, farSplitDistance, frustumCornersWS);
	// calc center of the frustum
	glm::vec3 frustumCenter = glm::vec3(0.0f);
	for (unsigned int i = 0; i < 8; ++i)
		frustumCenter += frustumCornersWS[i];
	frustumCenter /= 8.0f;

	// calc sphere that tightly encloses the frustum
	// We use the max distance from the frustum center to the corners
	// TODO Actually should the distance ot all corners be the same???
	float radius = 0.0f;
	for (unsigned int i = 0; i < 8; ++i)
	{
		float distance = glm::length(frustumCornersWS[i] - frustumCenter);
		radius = std::max<float>(radius, distance);
	}
	radius = std::ceil(radius * 16.0f) / 16.0f; // alignment???



	return { frustumCenter, radius };
}

void CascadedShadow::extractFrustumPoints(Camera* camera, float nearSplitDistance, float farSplitDistance, glm::vec3(& frustumCorners)[8])
{
	// old stuff
	/*const auto& position = camera->getPosition();
	const auto& up = camera->getUp(); // glm::vec3(0,1,0);
	const auto& look = camera->getLook(); // glm::vec3(0,0,1);
	const auto& right = camera->getRight(); // glm::vec3(-1,0,0);

	const float aspectRatio = camera->getAspectRatio();
	const float fov = glm::radians(camera->getFOV());

	// Calculate the tangent values (this can be cached
	const float tanFOVX = tanf(aspectRatio * fov / 2.0f);
	const float tanFOVY = tanf(fov / 2.0f);



	// Calculate the points on the near plane
	frustumCorners[0] = position + (-right * tanFOVX + up * tanFOVY + look) * nearPlane;
	frustumCorners[1] = position + (right * tanFOVX + up * tanFOVY + look) * nearPlane;
	frustumCorners[2] = position + (right * tanFOVX - up * tanFOVY + look) * nearPlane;
	frustumCorners[3] = position + (-right * tanFOVX - up * tanFOVY + look) * nearPlane;

	// Calculate the points on the far plane
	frustumCorners[4] = position + (-right * tanFOVX + up * tanFOVY + look) * farPlane;
	frustumCorners[5] = position + (right * tanFOVX + up * tanFOVY + look) * farPlane;
	frustumCorners[6] = position + (right * tanFOVX - up * tanFOVY + look) * farPlane;
	frustumCorners[7] = position + (-right * tanFOVX - up * tanFOVY + look) * farPlane;*/



	// At first we define the frustum corners in NDC space
	glm::vec3 frustumCornersWS[8] =
	{
		glm::vec3(-1.0f,  1.0f, -1.0f),
		glm::vec3(1.0f,  1.0f, -1.0f),
		glm::vec3(1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f,  1.0f,  1.0f),
		glm::vec3(1.0f,  1.0f,  1.0f),
		glm::vec3(1.0f, -1.0f,  1.0f),
		glm::vec3(-1.0f, -1.0f,  1.0f),
	};

	// Now we transform the frustum corners back to world space
	glm::mat4 invViewProj = glm::inverse(camera->getPerspProjection() * camera->getView());
	for (unsigned int i = 0; i < 8; ++i)
	{
		glm::vec4 inversePoint = invViewProj * glm::vec4(frustumCornersWS[i], 1.0f);
		frustumCornersWS[i] = glm::vec3(inversePoint / inversePoint.w);
	}

	// Calculate rays that define the near and far plane of each cascade split.
	// Than we translate the frustum corner accordingly so that the frustum starts at the near splitting plane
	// and ends at the far splitting plane.
	for (unsigned int i = 0; i < 4; ++i)
	{
		glm::vec3 cornerRay = frustumCornersWS[i + 4] - frustumCornersWS[i];
		glm::vec3 nearCornerRay = cornerRay * nearSplitDistance;
		glm::vec3 farCornerRay = cornerRay * farSplitDistance;
		frustumCorners[i + 4] = frustumCornersWS[i] + farCornerRay;
		frustumCorners[i] = frustumCornersWS[i] + nearCornerRay;
	}
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

CascadedShadow::DepthPassShader::DepthPassShader()
{
	mProgram = ShaderProgram::create("CascadedShadows/shadowDepthPass_vs.glsl", "CascadedShadows/shadowDepthPass_fs.glsl");
}

void CascadedShadow::DepthPassShader::onModelMatrixUpdate(const glm::mat4& modelMatrix)
{
	static const UniformLocation MODEL_MATRIX_LOCATION = 1;
	mProgram->setMat4(MODEL_MATRIX_LOCATION, modelMatrix);
	//glUniformMatrix4fv(MODEL_MATRIX_LOCATION, 1, GL_FALSE, &(*modelMatrix)[0][0]);
}


void CascadedShadow::frameUpdate(Camera* camera, const glm::vec3& lightDirection)
{
	frameUpdateNew(camera, lightDirection);
	//frameUpdateOld(camera, lightDirection);
}

Shader* CascadedShadow::getDepthPassShader()
{
	return &mDepthPassShader;
}

CascadedShadow::CascadeData* CascadedShadow::getCascadeData()
{
	return &mCascadeData;
}