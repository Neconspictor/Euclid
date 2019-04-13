#pragma once
#include <nex/shader/Shader.hpp>

namespace nex
{
	class Material;

	class Pass
	{
	public:

		Pass(std::unique_ptr<Shader> shader = nullptr);

		/**
		 * base class needs virtual destructor. Rule of five.
		 */
		virtual ~Pass() = default;
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

		virtual void onModelMatrixUpdate(const glm::mat4& modelMatrix);

		virtual void onMaterialUpdate(const Material* material);

		// Function that should be called before render calls
		virtual void setupRenderState();

		// Reverse the state of the function setupRenderState
		// TODO
		virtual void reverseRenderState();

		virtual void updateConstants();

	protected:

		std::unique_ptr<Shader> mShader;

		// Many passes need a sampler object, so we specify one default one here.
		Sampler mSampler;
	};

	class TransformPass : public Pass
	{
	public:
		TransformPass(std::unique_ptr<Shader> program = nullptr);
		
		virtual ~TransformPass() = default;
		TransformPass(const TransformPass&) = delete;
		TransformPass(TransformPass&&) = default;
		TransformPass& operator=(const TransformPass&) = delete;
		TransformPass& operator=(TransformPass&&) = default;

		virtual void onTransformUpdate(const TransformData& data) = 0;
	};

	class ComputePass : public Pass
	{
	public:
		ComputePass(std::unique_ptr<Shader> program = nullptr);

		virtual ~ComputePass() = default;
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