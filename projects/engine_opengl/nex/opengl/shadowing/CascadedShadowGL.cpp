#include <nex/opengl/shadowing/CascadedShadowGL.hpp>
#include <glm/gtc/matrix_transform.inl>
#include "nex/opengl/texture/TextureGL.hpp"
#include "nex/mesh/VertexArray.hpp"
#include "nex/mesh/IndexBuffer.hpp"
#include "nex/opengl/opengl.hpp"
#include "nex/mesh/SubMesh.hpp"

using namespace nex;

CascadedShadowGL::CascadedShadowGL(unsigned int cascadeWidth, unsigned int cascadeHeight) :
	mCascadeWidth(cascadeWidth),
	mCascadeHeight(cascadeHeight),
	mShadowMapSize(std::max<int>(cascadeWidth, cascadeHeight))
{
	// Directional light shadow map buffer
	glGenFramebuffers(1, &mCascadedShadowFBO);


	mDepthPass = ShaderProgram::create("CascadedShadows/shadowDepthPass_vs.glsl", "CascadedShadows/shadowDepthPass_fs.glsl");

	mDepthTextureArray = std::make_unique<Texture2D>(std::make_unique<Texture2DGL>(GL_FALSE, TextureData(), cascadeWidth, cascadeHeight));

	updateTextureArray();
}

CascadedShadowGL::~CascadedShadowGL()
{
	if (mCascadedShadowFBO != GL_FALSE)
		glDeleteBuffers(1, &mCascadedShadowFBO);
	mCascadedShadowFBO = GL_FALSE;
}

void CascadedShadowGL::begin(int cascadeIndex)
{
	mDepthPass->bind();

	glBindFramebuffer(GL_FRAMEBUFFER, mCascadedShadowFBO);
	glScissor(0, 0, mCascadeWidth, mCascadeWidth);
	glViewport(0, 0, mCascadeWidth, mCascadeWidth);

	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *((TextureGL*)mDepthTextureArray->getImpl())->getTexture(), 0, cascadeIndex);

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_CLAMP); // We use depth clamping so that the shadow maps keep from moving through objects which causes shadows to disappear.
	glCullFace(GL_BACK);

	glm::mat4 lightViewProjection = mCascadeData.lightViewProjectionMatrices[cascadeIndex];

	// update lightViewProjectionMatrix uniform
	static const GLuint LIGHT_VIEW_PROJECTION_MATRIX_LOCATION = 0;
	glUniformMatrix4fv(LIGHT_VIEW_PROJECTION_MATRIX_LOCATION, 1, GL_FALSE, &lightViewProjection[0][0]);
}

void CascadedShadowGL::end()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	mDepthPass->unbind();
	//glDisable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_CLAMP);
	glCullFace(GL_BACK);
}

Texture* CascadedShadowGL::getDepthTextureArray()
{
	return mDepthTextureArray.get();
}

void CascadedShadowGL::resize(unsigned cascadeWidth, unsigned cascadeHeight)
{
	mCascadeWidth = cascadeWidth;
	mCascadeHeight = cascadeHeight;
	mShadowMapSize = std::max<int>(cascadeWidth, cascadeHeight);
	updateTextureArray();
}

void CascadedShadowGL::render(SubMesh* mesh, const glm::mat4* modelMatrix)
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
}

void CascadedShadowGL::updateTextureArray()
{
	TextureGL* depthGL = (TextureGL*)mDepthTextureArray->getImpl();
	depthGL->release();

	glBindFramebuffer(GL_FRAMEBUFFER, mCascadedShadowFBO);

	GLuint temp = GL_FALSE;
	glGenTextures(1, &temp);
	depthGL->setTexture(temp);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, *depthGL->getTexture());
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, mCascadeWidth, mCascadeHeight, NUM_CASCADES, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE); //GL_COMPARE_REF_TO_TEXTURE
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	//glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depthGL->getTexture(), 0);

	GLCall(glDrawBuffer(GL_NONE));
	GLCall(glReadBuffer(GL_NONE));
	//GLCall(glNamedFramebufferDrawBuffer(mCascadedShadowFBO, GL_NONE));
	//GLCall(glNamedFramebufferReadBuffer(mCascadedShadowFBO, GL_NONE));


	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}


void CascadedShadowGL::frameUpdate(Camera* camera, const glm::vec3& lightDirection)
{
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
		mLightViewMatrix = glm::mat4(1.0f);
		mLightViewMatrix = glm::lookAt(lightPositionWS, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));

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
		const float clipDist = clipRange;
		mCascadeData.cascadedSplits[cascadeIterator].x = (nearClip + splitDistance * clipDist) * -1.0f;
		mCascadeData.lightViewProjectionMatrices[cascadeIterator] = mLightProjMatrix * mLightViewMatrix;
		mCascadeData.inverseViewMatrix = inverse(camera->getView());
	}
}

const glm::mat4& CascadedShadowGL::getLightProjectionMatrix() const
{
	return mLightProjMatrix;
}

CascadedShadowGL::CascadeData* CascadedShadowGL::getCascadeData()
{
	return &mCascadeData;
}