#pragma once
#include <nex/shader/Shader.hpp>
#include <nex/material/Material.hpp>
#include <nex/texture/Sampler.hpp>
#include <nex/material/AbstractMaterialLoader.hpp>
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
		void setTime(float time);

		void updateConstants(const RenderContext& constants) override;
		void updateMaterial(const Material& material) override;
	protected:
		UniformTex mStructure;
		Uniform mBaseColor;
		Uniform mTime;
	};


	class FlameMaterial : public Material {
	public:
		FlameMaterial(std::shared_ptr<ShaderProvider> provider, const Texture* structure,
			std::unique_ptr<Sampler> structureSampler,
			const glm::vec4& baseColor);
		virtual ~FlameMaterial() = default;

		const Texture* structure;
		std::unique_ptr<Sampler> structureSampler;
		glm::vec4 baseColor;
	};

	class FlameMaterialLoader : public AbstractMaterialLoader {
	public:

		FlameMaterialLoader(
			FlameShader* shader,
			const Texture* structure, 
			const SamplerDesc& structureSamplerDesc, 
			const glm::vec4& baseColor);
		virtual ~FlameMaterialLoader() = default;

		void loadShadingMaterial(const std::filesystem::path& meshPathAbsolute, 
			const aiScene* scene, MaterialStore& store, unsigned materialIndex) const override;

		std::unique_ptr<Material> createMaterial(const MaterialStore& store) const override;

	private:
		FlameShader* mShader;
		std::shared_ptr<ShaderProvider> mProvider;
		const Texture* mStructure; 
		SamplerDesc mStructureSamplerDesc;
		glm::vec4 mBaseColor;
	};
}