#pragma once
#include <nex/shader/Shader.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/texture/RenderTarget.hpp>

namespace nex
{
	/**
 * Abstract class for Cascaded shadow implementations.
 */
	class CascadedShadowGL
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

		CascadedShadowGL(unsigned int cascadeWidth, unsigned int cascadeHeight);

		/**
		 * Allows rendering to the i-th cascade.
		 */
		void begin(int cascadeIndex);

		/**
		 * Finishes rendering to the i-th shadow cascade.
		 * Should be called after rendering to the cascade
		 * Has to be called AFTER CascadedShadow::begin(int)
		 */
		void end();

		nex::Texture* getDepthTextureArray();

		/**
		 * Resizes the cascades
		 */
		void resize(unsigned int cascadeWidth, unsigned int cascadeHeight);

		/**
		 * Renders a mesh with a given model matrix to the active cascade
		 */
		//void render(SubMesh* mesh, const glm::mat4* modelMatrix);

		/**
		 * Updates the cascades. Has to be called once per frame and before actual renering to the cascades happens.
		 */
		void frameUpdate(Camera* camera, const glm::vec3& lightDirection);

		CascadeData* getCascadeData();

		const glm::mat4& getLightProjectionMatrix() const;
		Shader* getDepthPassShader();


	protected:

		void updateTextureArray();
	protected:

		class DepthPassShader : public Shader
		{
		public:
			DepthPassShader();
			void onModelMatrixUpdate(const glm::mat4& modelMatrix) override;
		};


		DepthPassShader mDepthPassShader;
		RenderTarget mRenderTarget;

	protected:
		glm::mat4 mLightViewMatrix;
		glm::mat4 mLightProjMatrix;

		unsigned int mCascadeWidth;
		unsigned int mCascadeHeight;

		float mShadowMapSize;
		CascadeData mCascadeData;
	};
}