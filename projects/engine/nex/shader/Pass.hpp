#pragma once
#include <nex/shader/Shader.hpp>
#include <nex/buffer/ShaderBuffer.hpp>

namespace nex
{
	class Camera;
	class Material;

	class Pass
	{
	public:

		struct Constants 
		{
			const Camera* camera;
			unsigned windowWidth; 
			unsigned windowHeight;
		};

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

		/**
		 * Checks, if the pass is currently bound.
		 */
		bool isBound()const;

		void setShader(std::unique_ptr<Shader> shader);

		/**
		 * Unbinds this shader and the underlying shader program.
		 */
		void unbind();

		virtual void updateConstants(const Constants& constants);

	protected:

		std::unique_ptr<Shader> mShader;
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
			glm::mat4 prevTransform;
			glm::mat4 modelView;
			glm::mat4 normalMatrix; // actually a mat3, but std140 layout extends it to a mat4, so we have to use that.
		};

		/**
		 * Every shader used by a TransformPass has to have a Transforms uniform buffer on this binding point.
		 */
		static const unsigned TRANSFORM_BUFFER_BINDING_POINT = 0;

		TransformPass(std::unique_ptr<Shader> program = nullptr, unsigned transformBindingPoint = 0);

		virtual ~TransformPass();
		TransformPass(const TransformPass&) = delete;
		TransformPass(TransformPass&&) = default;
		TransformPass& operator=(const TransformPass&) = delete;
		TransformPass& operator=(TransformPass&&) = default;

		/**
		 * Sets the current and the previous model matrix (from the last frame)
		 * Note: setViewProjectionMatrices has to be called before calling this function!
		 */
		void setModelMatrix(const glm::mat4& model, const glm::mat4& prevModel);

		/**
		 * Sets the projection matrix, the current view matrix and the previous view matrix (from the last frame)
		 */
		void setViewProjectionMatrices(const glm::mat4& projection, const glm::mat4& view, const glm::mat4& prevView, const glm::mat4& prevViewProj);

		/**
		 * Note: setViewProjectionMatrices and setModelMatrix have to be called before calling this function!
		 * Pass has to be bound.
		 */
		void uploadTransformMatrices();

	protected:
		unsigned mTransformBindingPoint;
		ShaderStorageBuffer mTransformBuffer;
		Transforms mTransforms;
		glm::mat4 mPrevModel;
		glm::mat4 mPrevView;
		glm::mat4 mPrevViewProjection;

	};


	class SimpleTransformPass : public Pass
	{
	public:
		SimpleTransformPass(std::unique_ptr<Shader> program = nullptr, unsigned transformLocation = 0);

		virtual ~SimpleTransformPass();
		SimpleTransformPass(const SimpleTransformPass&) = delete;
		SimpleTransformPass(SimpleTransformPass&&) = default;
		SimpleTransformPass& operator=(const SimpleTransformPass&) = delete;
		SimpleTransformPass& operator=(SimpleTransformPass&&) = default;

		/**
		 * Sets the current and the previous model matrix (from the last frame)
		 * Note: updateViewProjection has to be called before calling this function!
		 * NOTE: Pass has to be bound!
		 */
		void updateTransformMatrix(const glm::mat4& model);

		/**
		 * Sets the view-projection matrix used to form the final transformation matrix when combined with a model matrix.
		 */
		void updateViewProjection(const glm::mat4& projection, const glm::mat4& view);

	protected:
		glm::mat4 mViewProjection;
		unsigned mTransformLocation;
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