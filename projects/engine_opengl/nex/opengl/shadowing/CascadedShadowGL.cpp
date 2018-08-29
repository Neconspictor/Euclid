#include <nex/opengl/shadowing/CascadedShadowGL.hpp>
#include <nex/opengl/mesh/MeshGL.hpp>

CascadedShadowGL::CascadedShadowGL(unsigned int cascadeWidth, unsigned int cascadeHeight) :
	CascadedShadow(cascadeWidth, cascadeHeight), mDepthPass("CascadedShadows/shadowDepthPass_vs.glsl", "CascadedShadows/shadowDepthPass_fs.glsl")
{
	// Directional light shadow map buffer
	glGenFramebuffers(1, &mCascadedShadowFBO);


	updateTextureArray();
}

CascadedShadowGL::~CascadedShadowGL()
{
	if (mCascadedShadowFBO != GL_FALSE)
		glDeleteBuffers(1, &mCascadedShadowFBO);
	mCascadedShadowFBO = GL_FALSE;

	releaseTextureArray();
}

void CascadedShadowGL::begin(int cascadeIndex)
{
	mDepthPass.use();

	glBindFramebuffer(GL_FRAMEBUFFER, mCascadedShadowFBO);
	glViewport(0, 0, mCascadeWidth, mCascadeHeight);

	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mCascadedTextureArray, 0, cascadeIndex);

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_CLAMP);
	glCullFace(GL_FRONT);

	glm::mat4 lightViewProjection = mLightOrthoMatrix * mLightViewMatrix;

	// update lightViewProjectionMatrix uniform
	static const GLuint LIGHT_VIEW_PROJECTION_MATRIX_LOCATION = 0;
	glUniformMatrix4fv(LIGHT_VIEW_PROJECTION_MATRIX_LOCATION, 1, GL_FALSE, &lightViewProjection[0][0]);
}

void CascadedShadowGL::end()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
	//glDisable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_CLAMP);
	glCullFace(GL_BACK);
}

void CascadedShadowGL::resize(unsigned cascadeWidth, unsigned cascadeHeight)
{
	CascadedShadow::resize(cascadeWidth, cascadeHeight);
	updateTextureArray();
}

void CascadedShadowGL::render(Mesh* mesh, glm::mat4* modelMatrix)
{
	MeshGL* meshGL = (MeshGL*)mesh;

	// Update modelMatrix uniform
	static const GLuint MODEL_MATRIX_LOCATION = 1;
	glUniformMatrix4fv(MODEL_MATRIX_LOCATION, 1, GL_FALSE, &(*modelMatrix)[0][0]);

	// render mesh
	glBindVertexArray(meshGL->getVertexArrayObject());
	GLsizei indexSize = static_cast<GLsizei>(meshGL->getIndexSize());
	glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void CascadedShadowGL::releaseTextureArray()
{
	if (mCascadedTextureArray != GL_FALSE)
		glDeleteTextures(1, &mCascadedTextureArray);
	mCascadedTextureArray = GL_FALSE;
}

void CascadedShadowGL::updateTextureArray()
{
	releaseTextureArray();

	glBindFramebuffer(GL_FRAMEBUFFER, mCascadedShadowFBO);

	glGenTextures(1, &mCascadedTextureArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, mCascadedTextureArray);

	glBindTexture(GL_TEXTURE_2D_ARRAY, mCascadedTextureArray);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, mCascadeWidth, mCascadeHeight, NUM_CASCADES, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mCascadedTextureArray, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}