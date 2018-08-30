#pragma once
#include <nex/shadowing/CascadedShadow.hpp>
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/opengl/texture/TextureGL.hpp>

class Camera;

/**
 * Abstract class for Cascaded shadow implementations.
 */
class CascadedShadowGL : public CascadedShadow
{
public:
	CascadedShadowGL(unsigned int cascadeWidth, unsigned int cascadeHeight);

	virtual ~CascadedShadowGL();

	/**
	 * Allows rendering to the i-th cascade.
	 */
	void begin(int cascadeIndex) override;

	/**
	 * Finishes rendering to the i-th shadow cascade.
	 * Should be called after rendering to the cascade
	 * Has to be called AFTER CascadedShadow::begin(int)
	 */
	void end() override;

	Texture* getDepthTextureArray() override;

	void resize(unsigned int cascadeWidth, unsigned int cascadeHeight) override;

	void render(Mesh* mesh, glm::mat4* modelMatrix) override;


protected:

	void updateTextureArray();
protected:
	ShaderGL mDepthPass;
	GLuint mCascadedShadowFBO = GL_FALSE;
	TextureGL mDepthTextureArray;
};