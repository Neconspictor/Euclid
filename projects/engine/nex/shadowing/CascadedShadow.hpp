#pragma once

class MeshGL;
class TexturGL;
class Camera;

/**
 * Abstract class for Cascaded shadow implementations.
 */
class CascadedShadow
{
public:

	/**
	 * Specifies the number of used cascades
	 * IMPORTANT: Keep in sync with shader implementation(s) 
	 */
	static const int NUM_CASCADES = 4;

	struct CascadeData {
		glm::mat4 inverseViewMatrix;
		glm::mat4 lightViewProjectionMatrices[NUM_CASCADES];
		glm::vec4 cascadedSplits[NUM_CASCADES];
	};

	CascadedShadow(unsigned int cascadeWidth, unsigned int cascadeHeight);

	virtual ~CascadedShadow() = default;

	/**
	 * Updates the cascades. Has to be called once per frame and before actual renering to the cascades happens.
	 */
	void frameUpdate(Camera* camera, const glm::vec3& lightDirection);

	const glm::mat4& getLightProjectionMatrix() const;

	/**
	 * Allows rendering to the i-th cascade.
	 */
	virtual void begin(int cascadeIndex) = 0;

	/**
	 * Finishes rendering to the i-th shadow cascade.
	 * Should be called after rendering to the cascade
	 * Has to be called AFTER CascadedShadow::begin(int)
	 */
	virtual void end() = 0;

	CascadeData* getCascadeData();

	virtual TextureGL* getDepthTextureArray() = 0;

	/**
	 * Resizes the cascades
	 */
	virtual void resize(unsigned int cascadeWidth, unsigned int cascadeHeight);

	/**
	 * Renders a mesh with a given model matrix to the active cascade
	 */
	virtual void render(MeshGL* mesh, glm::mat4* modelMatrix) = 0;

protected:
	glm::mat4 mLightViewMatrix;
	glm::mat4 mLightProjMatrix;

	unsigned int mCascadeWidth;
	unsigned int mCascadeHeight;

	float mShadowMapSize;
	CascadeData mCascadeData;
};