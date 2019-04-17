#pragma once
#include <nex/shader/Shader.hpp>
#include "ShaderBuffer.hpp"

namespace nex
{
	class Camera;
	class Material;

	class Pass
	{
	public:

		Pass(std::unique_ptr<Shader> shader = nullptr);

		/**
		 * base class needs virtual destructor. Rule of five.
		 */
		virtual ~Pass();
		Pass(const Pass&) = delete;
		Pass(Pass&&) = default;
		Pass& operator=(const Pass&) = delete;
		Pass& operator=(Pass&&) = default;

		/**
		 * Binds this shader and the underlying shader program.
		 */
		void bind();

		Shader* getShader();

		void setShader(std::unique_ptr<Shader> shader);

		/**
		 * Unbinds this shader and the underlying shader program.
		 */
		void unbind();

		// Function that should be called before render calls
		virtual void setupRenderState();

		// Reverse the state of the function setupRenderState
		// TODO
		virtual void reverseRenderState();

		virtual void updateConstants(Camera* camera);

	protected:

		std::unique_ptr<Shader> mShader;

		// Many passes need a sampler object, so we specify one default one here.
		Sampler mSampler;
	};

	class TransformPass : public Pass
	{
	public:

		struct Transforms
		{
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 projection;
			glm::mat4 transform;
			glm::mat4 modelView;
			glm::mat3 normalMatrix;
		};

		/**
		 * Every shader used by a TransformPass has to have a Transforms uniform buffer on this binding point.
		 */
		static const unsigned TRANSFORM_BUFFER_BINDING_POINT = 0;

		TransformPass(std::unique_ptr<Shader> program = nullptr);
		
		virtual ~TransformPass();
		TransformPass(const TransformPass&) = delete;
		TransformPass(TransformPass&&) = default;
		TransformPass& operator=(const TransformPass&) = delete;
		TransformPass& operator=(TransformPass&&) = default;

		void setViewProjectionMatrices(const glm::mat4& projection, const glm::mat4& view);

		/**
		 * Note: setViewProjectionMatrices has to be called before calling this function!
		 */
		void setModelMatrix(const glm::mat4& model);

		/**
		 * Note: setViewProjectionMatrices and setModelMatrix have to be called before calling this function!
		 * Pass has to be bound.
		 */
		void uploadTransformMatrices();

	protected:
		ShaderStorageBuffer mTransformBuffer;
		Transforms mTransforms;
	};

	class ComputePass : public Pass
	{
	public:
		ComputePass(std::unique_ptr<Shader> program = nullptr);

		virtual ~ComputePass();
		ComputePass(const ComputePass&) = delete;
		ComputePass(ComputePass&&) = default;
		ComputePass& operator=(const ComputePass&) = delete;
		ComputePass& operator=(ComputePass&&) = default;

		/**
		 * Notice: Has to be implemented by the render backend implementation!
		 * Notice: The shader has to be bound (with bind()) before this function is called!
		 * Otherwise the behaviour is undefined!
		 * 
		 * @param workGroupsX: The number of work groups to be launched in the X dimension. 
		 * @param workGroupsY: The number of work groups to be launched in the Y dimension. 
		 * @param workGroupsZ: The number of work groups to be launched in the Z dimension. 
		 */
		void dispatch(unsigned workGroupsX, unsigned workGroupsY, unsigned workGroupsZ);
	};

	std::ostream& operator<<(std::ostream& os, nex::ShaderStageType stageType);
};