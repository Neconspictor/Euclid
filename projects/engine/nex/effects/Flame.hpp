#pragma once
#include <nex/shader/Shader.hpp>
#include <nex/material/Material.hpp>
#include <nex/texture/Sampler.hpp>
#include <memory>

namespace nex
{
	class FlameShader : public TransformShader
	{
	public:
		FlameShader();

		virtual ~FlameShader() = default;

		void setStructure(const Texture* structure, const Sampler* sampler);
		void setBaseColor(const glm::vec4& color);

		void upload(const Material& material) override;
	protected:
		UniformTex mStructure;
		Uniform mBaseColor;
	};


	class FlameMaterial : public Material {
	public:
		FlameMaterial(FlameShader* shader, const Texture* structure, 
			std::unique_ptr<Sampler> structureSampler,
			const glm::vec4& baseColor);
		virtual ~FlameMaterial() = default;

		const Texture* structure;
		std::unique_ptr<Sampler> structureSampler;
		glm::vec4 baseColor;
	};
}