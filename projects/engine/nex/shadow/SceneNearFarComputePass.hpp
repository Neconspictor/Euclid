#pragma once
#include <nex/shader/Shader.hpp>

namespace nex
{

	class ShaderStorageBuffer;

	class SceneNearFarComputePass : public nex::ComputeShader
	{
	public:
		struct Constant
		{
			// Holds positive(!) camera view near and far z-value
			// z and w component aren't used
			glm::vec4 mCameraNearFar;
			glm::mat4 mCameraProj;
		};

		struct WriteOut
		{
			// x and y component will hold the positive(!) min and max z value; the other components are not used
			glm::vec4 minMax;
			
		};

		SceneNearFarComputePass();

		/**
		 * Note: This function only works correct, if this shader is bound!
		 */
		WriteOut readResult();

		/**
		 * Note: This function only works correct, if this shader is bound!
		 */
		void reset();

		/**
		 * Note: This function only works correct, if this shader is bound!
		 */
		void setConstants(float positiveViewNearZ, float positiveViewFarZ, const glm::mat4& projection);

		/**
		 * Note: This function only works correct, if this shader is bound!
		 */
		void setDepthTexture(Texture* depth);

		ShaderStorageBuffer* getConstantBuffer();
		ShaderStorageBuffer* getWriteOutBuffer();

	private:
		std::unique_ptr<ShaderStorageBuffer> mConstantBuffer;
		std::unique_ptr<ShaderStorageBuffer> mWriteOutBuffer;
		UniformTex mDepthTextureUniform;
	};
}