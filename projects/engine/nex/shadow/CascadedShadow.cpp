#include <nex/shadow/CascadedShadow.hpp>
#include <nex/mesh/SubMesh.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/RenderBackend.hpp>
#include <glm/gtc/matrix_transform.inl>
using namespace nex;

CascadedShadow::CascadedShadow(unsigned int cascadeWidth, unsigned int cascadeHeight) :
	mCascadeWidth(cascadeWidth),
	mCascadeHeight(cascadeHeight),
	mShadowMapSize(std::max<int>(cascadeWidth, cascadeHeight)),
	mAntiFlickerOn(true)
{
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

void CascadedShadow::calcSplitSchemes(Camera* camera)
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
		const float splitDistance = (d - nearClip) / clipRange;

		mCascadeData.cascadedFarPlanesVS[i].x = (nearClip + splitDistance * clipRange) * -1.0f; // -1.0f for right handed coord systems


	}
}

CascadedShadow::BoundingSphere CascadedShadow::extractFrustumBoundSphere(Camera* camera, float nearPlane, float farPlane)
{
	const auto& position = camera->getPosition();
	const auto& up = camera->getUp();
	const auto& look = camera->getLook();
	const auto& right = camera->getRight();


	const float aspectRatio = glm::radians(camera->getAspectRatio());
	const float fov = glm::radians(camera->getFOV());

	// Calculate the tangent values (this can be cached as long as the FOV doesn't change)
	const float tanFOVX = tanf(aspectRatio * fov);
	const float tanFOVY = tanf(aspectRatio);

	// The center of the sphere is in the center of the frustum
	const auto boundCenter = position + look * (nearPlane + 0.5f * (nearPlane + farPlane));

	// Radius is the distance to one of the frustum far corners
	// TODO use right instead of -right ???
	const auto boundSpan = position + (-right * tanFOVX + up * tanFOVY + look) * farPlane - boundCenter;
	const auto boundRadius = glm::length(boundSpan);

	return { boundCenter, boundRadius };
}

void CascadedShadow::extractFrustumPoints(Camera* camera, float nearPlane, float farPlane, glm::vec3(& frustumCorners)[8])
{
	const auto& position = camera->getPosition();
	const auto& up = glm::vec3(0,1,0);//camera->getUp();
	const auto& look = glm::vec3(0,0,-1);//camera->getLook();
	const auto& left = glm::vec3(1,0,0);//camera->getRight();

	const float aspectRatio = glm::radians(camera->getAspectRatio());
	const float fov = glm::radians(camera->getFOV());

	// Calculate the tangent values (this can be cached
	const float tanFOVX = camera->getFrustum(Perspective).right * 2;//tanf(aspectRatio * fov / 2.0f);
	const float tanFOVY = camera->getFrustum(Perspective).top * 2;//tanf(aspectRatio / 2.0f);


	// Calculate the points on the near plane
	frustumCorners[0] = (-left * tanFOVX + up * tanFOVY + look) * nearPlane;
	frustumCorners[1] = (left * tanFOVX + up * tanFOVY + look) * nearPlane;
	frustumCorners[2] = (left * tanFOVX - up * tanFOVY + look) * nearPlane;
	frustumCorners[3] = (-left * tanFOVX - up * tanFOVY + look) * nearPlane;

	// Calculate the points on the far plane
	frustumCorners[4] = (-left * tanFOVX + up * tanFOVY + look) * farPlane;
	frustumCorners[5] = (left * tanFOVX + up * tanFOVY + look) * farPlane;
	frustumCorners[6] = (left * tanFOVX - up * tanFOVY + look) * farPlane;
	frustumCorners[7] = (-left * tanFOVX - up * tanFOVY + look) * farPlane;



	/*// At first we define the frustum corners in NDC space
	const glm::vec3 frustumCornersNDC[8] =
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
		glm::vec4 inversePoint = invViewProj * glm::vec4(frustumCornersNDC[i], 1.0f);
		frustumCorners[i] = glm::vec3(inversePoint / inversePoint.w);
	}

	// Calculate rays that define the near and far plane of each cascade split.
	// Than we translate the frustum corner accordingly so that the frustum starts at the near splitting plane
	// and ends at the far splitting plane.
	for (unsigned int i = 0; i < 4; ++i)
	{
		glm::vec3 cornerRay = frustumCorners[i + 4] - frustumCorners[i];
		glm::vec3 nearCornerRay = cornerRay * prevSplitDistance;
		glm::vec3 farCornerRay = cornerRay * splitDistance;
		frustumCorners[i + 4] = frustumCorners[i] + farCornerRay;
		frustumCorners[i] = frustumCorners[i] + nearCornerRay;
	}*/

}

bool CascadedShadow::cascadeNeedsUpdate(const glm::mat4& shadowView, int cascadeIdx, const glm::vec3& newCenter,
	glm::vec3* offset)
{
	// Find the offset between the new and old bound ceter
	glm::vec3 oldCenterInCascade = glm::vec3(shadowView * glm::vec4(m_arrCascadeBoundCenter[cascadeIdx], 0));
	glm::vec3 newCenterInCascade = glm::vec3(shadowView * glm::vec4(newCenter, 0));
	const glm::vec3 centerDiff = oldCenterInCascade - newCenterInCascade;

	// Find the pixel size based on the diameters and map pixel size
	const float pixelSize = (float)mShadowMapSize / (2.0f * m_arrCascadeBoundRadius[cascadeIdx]);

	const float pixelOffX = centerDiff.x * pixelSize;
	const float pixelOffY = centerDiff.y * pixelSize;

	// Check if the center moved at least half a pixel unit
	const bool needUpdate = abs(pixelOffX) > 0.5f || abs(pixelOffY) > 0.5f;
	if (needUpdate)
	{
		// Round to the 
		offset->x = floorf(0.5f + pixelOffX) / pixelSize;
		offset->y = floorf(0.5f + pixelOffY) / pixelSize;
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
	const float minDistance = 0.0f;
	const float cameraNearPlaneVS = -camera->getFrustum(ProjectionMode::Perspective).nearPlane;
	calcSplitSchemes(camera);
	mCascadeData.inverseViewMatrix = inverse(camera->getView());

	for (unsigned int cascadeIterator = 0; cascadeIterator < NUM_CASCADES; ++cascadeIterator)
	{

		//float prevSplitDistance = cascadeIterator == 0 ? minDistance : mCascadeData.cascadedSplits[cascadeIterator - 1].x;
		//float splitDistance = cascadeSplits[cascadeIterator];

		// the far plane of the previous cascade is the near plane of the current cascade
		const float nearPlane = cascadeIterator == 0 ? cameraNearPlaneVS : mCascadeData.cascadedFarPlanesVS[cascadeIterator - 1].x;
		float farPlane = mCascadeData.cascadedFarPlanesVS[cascadeIterator].x;

		glm::vec3 frustumCornersVS[8];
		// extracts frustum points in view space
		extractFrustumPoints(camera, nearPlane, farPlane, frustumCornersVS);

		// calc center of the frustum
		glm::vec3 frustumCornersWS[8];
		glm::vec3 frustumCenterWS = glm::vec3(0.0f);
		for (unsigned int i = 0; i < 8; ++i)
		{
			// transform point to world space and add it to center
			frustumCornersWS[i] = mCascadeData.inverseViewMatrix * glm::vec4(frustumCornersVS[i], 1.0f);
			frustumCenterWS += frustumCornersWS[i];
		}
			
		frustumCenterWS /= 8.0f;

		//float far = -INFINITY;
		//float near = INFINITY;

		// calc sphere that tightly encloses the frustum
		// We use the max distance from the frustum center to the corners
		// TODO Actually should the distance ot all corners be the same???
		float radius = 0.0f;
		for (unsigned int i = 0; i < 8; ++i)
		{
			float distance = glm::length(frustumCornersWS[i] - frustumCenterWS);
			radius = std::max<float>(radius, distance);
		}
		radius = std::ceil(radius * 16.0f) / 16.0f; // alignment???

		glm::vec3 maxExtents = glm::vec3(radius, radius, radius);
		glm::vec3 minExtents = -maxExtents;
		glm::vec3 cascadeExtents = maxExtents - minExtents;


		//Position the view matrix looking down the center of the frustum with the light direction
		// We multiply the normalized light direction by radius so that nothing of the shadow map gets clipped
		glm::vec3 lightPositionWS = frustumCenterWS - normalize(lightDirection) * radius;
		mLightViewMatrix = glm::mat4(1.0f);
		mLightViewMatrix = glm::lookAt(lightPositionWS, frustumCenterWS, glm::vec3(0.0f, 1.0f, 0.0f));

		// Calculate the projection matrix by defining the AABB of the cascade (as ortho projection)
		mLightProjMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, cascadeExtents.z);

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
		//const float clipDist = clipRange;
		//mCascadeData.cascadedSplits[cascadeIterator].x = (nearClip + splitDistance * clipDist) * -1.0f;
		mCascadeData.lightViewProjectionMatrices[cascadeIterator] = mLightProjMatrix * mLightViewMatrix;
	}
}

const glm::mat4& CascadedShadow::getLightProjectionMatrix() const
{
	return mLightProjMatrix;
}

Shader* CascadedShadow::getDepthPassShader()
{
	return &mDepthPassShader;
}

CascadedShadow::CascadeData* CascadedShadow::getCascadeData()
{
	return &mCascadeData;
}