#pragma once
#include <nex/shader/ShaderProgram.hpp>
#include <nex/buffer/ShaderBuffer.hpp>
#include <nex/renderer/RenderContext.hpp>
#include <interface/buffers.h>

namespace nex
{
	class Material;
	struct RenderCommand;

	template<class ShaderType>
	struct ShaderOverride {
		ShaderType* default = nullptr;
		ShaderType* rigged = nullptr;
	};

	class Shader
	{
	public:

		static constexpr unsigned DEFAULT_BONE_BUFFER_BINDING_POINT = 1;

		Shader(std::unique_ptr<ShaderProgram> program = nullptr);

		/**
		 * base class needs virtual destructor. Rule of five.
		 */
		virtual ~Shader();
		Shader(const Shader&) = delete;
		Shader(Shader&&) = default;
		Shader& operator=(const Shader&) = delete;
		Shader& operator=(Shader&&) = default;

		/**
		 * Binds this shader and the underlying shader program.
		 */
		void bind();

		virtual void bindBoneTrafoBuffer(ShaderStorageBuffer* buffer) const;

		ShaderProgram* getShader();

		/**
		 * Checks, if the pass is currently bound.
		 */
		bool isBound()const;

		void setProgram(std::unique_ptr<ShaderProgram> program);

		/**
		 * Unbinds this shader and the underlying shader program.
		 */
		void unbind();

		/**
		 * Updates the shader with per frame constants.
		 */
		virtual void updateConstants(const RenderContext& constants);

		/**
		 * Updates the shader with per instance data
		 */
		virtual void updateInstance(
			const RenderContext& constants,
			const RenderCommand& command,
			const void* data = nullptr);

		/**
		 * Configures the shader with material data.
		 */
		virtual void updateMaterial(const Material& material);

	protected:

		std::unique_ptr<ShaderProgram> mProgram;
	};

	/**
	 * A special shader class that expects a bound ShaderConstants buffer and a bound PerObjectData buffer (from interface/buffers.h)
	 */
	class TransformShader : public Shader
	{
	public:

		TransformShader(std::unique_ptr<ShaderProgram> program = nullptr);

		virtual ~TransformShader();
		TransformShader(const TransformShader&) = delete;
		TransformShader(TransformShader&&) = default;
		TransformShader& operator=(const TransformShader&) = delete;
		TransformShader& operator=(TransformShader&&) = default;

		/**
		 * Note: setViewProjectionMatrices and setModelMatrix have to be called before calling this function!
		 * Shader has to be bound.
		 */
		void uploadTransformMatrices(const RenderContext& constants, const RenderCommand& command);

		void updateConstants(const RenderContext& constants) override;
		void updateInstance(const RenderContext& constants, 
			const RenderCommand& command,
			const void* data = nullptr) override;

	protected:

		/**
		 * Sets the projection matrix, the current view matrix and the previous view matrix (from the last frame)
		 */
		void setViewProjectionMatrices(const glm::mat4& projection, 
			const glm::mat4& view, 
			const glm::mat4& invView,
			const glm::mat4& prevView, 
			const glm::mat4& prevViewProj);

		glm::mat4 mPrevView;
		glm::mat4 mPrevViewProjection;
		glm::mat4 mView;
		glm::mat4 mProjection;

	};


	class SimpleTransformShader : public Shader
	{
	public:
		SimpleTransformShader(std::unique_ptr<ShaderProgram> program = nullptr, unsigned transformLocation = 0);

		virtual ~SimpleTransformShader();
		SimpleTransformShader(const SimpleTransformShader&) = delete;
		SimpleTransformShader(SimpleTransformShader&&) = default;
		SimpleTransformShader& operator=(const SimpleTransformShader&) = delete;
		SimpleTransformShader& operator=(SimpleTransformShader&&) = default;

		/**
		 * Sets the current and the previous model matrix (from the last frame)
		 * Note: updateViewProjection has to be called before calling this function!
		 * NOTE: Shader has to be bound!
		 */
		void updateTransformMatrix(const glm::mat4& model);

		/**
		 * Sets the view-projection matrix used to form the final transformation matrix when combined with a model matrix.
		 */
		void updateViewProjection(const glm::mat4& projection, const glm::mat4& view);

		void updateConstants(const RenderContext& constants) override;
		void updateInstance(const glm::mat4& model, const glm::mat4& prevModel);

	protected:
		glm::mat4 mViewProjection;
		unsigned mTransformLocation;
	};


	class ComputeShader : public Shader
	{
	public:
		ComputeShader(std::unique_ptr<ShaderProgram> program = nullptr);

		virtual ~ComputeShader();
		ComputeShader(const ComputeShader&) = delete;
		ComputeShader(ComputeShader&&) = default;
		ComputeShader& operator=(const ComputeShader&) = delete;
		ComputeShader& operator=(ComputeShader&&) = default;

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