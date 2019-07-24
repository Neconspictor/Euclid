#pragma once
#include "nex/util/StringUtils.hpp"
#include <nex/shader/ShaderType.hpp>
#include <nex/renderer/RenderTypes.hpp>
#include <string>

namespace nex
{
	class Technique;
	class PbrTechnique;
	class TechniqueSelector;
	class Shader;
	class Texture;
	class BinStream;
	class Sampler;

	enum class MaterialType
	{
		Pbr,
		None
	};

	class Material
	{
	public:
		Material(Technique* technique);

		virtual ~Material();

		size_t getTypeHashCode() const;

		RenderState& getRenderState();
		Technique* getTechnique();

		void setTechnique(Technique* technique);

		virtual void upload();

	protected:
		Technique* mTechnique;
		RenderState mRenderState;
		size_t mHashCode;

		void setTypeHashCode(size_t code);
	};

	/**
		* Maps material enumerations to a string representation.
		*/
	static const util::EnumString<MaterialType> materialEnumConversion[] = {
		{ nex::MaterialType::Pbr, "PBR" },
		{ nex::MaterialType::None, "NONE" }
	};

	std::ostream& operator<<(std::ostream& os, nex::MaterialType type);


	class PbrMaterial : public Material
	{
	public:

		PbrMaterial(PbrTechnique* technique);
		PbrMaterial(
			PbrTechnique* technique,
			Texture* albedoMap,
			Texture* aoMap,
			Texture* emissionMap,
			Texture* metallicMap,
			Texture* normalMap,
			Texture* roughnessMap);

		const Texture* getAlbedoMap() const;
		const Texture* getAoMap() const;
		const Texture* getEmissionMap() const;
		const Texture* getMetallicMap() const;
		const Texture* getNormalMap() const;
		const Texture* getRoughnessMap() const;


		void setAlbedoMap(Texture* albedoMap);
		void setAoMap(Texture* aoMap);
		void setEmissionMap(Texture* emissionMap);
		void setMetallicMap(Texture* metallicMap);
		void setNormalMap(Texture* normalMap);
		void setRoughnessMap(Texture* roughnessMap);

		void upload() override;

	private:
		Texture* mAlbedoMap;
		Texture* mAoMap;
		Texture* mEmissionMap;
		Texture* mMetallicMap;
		Texture* mNormalMap;
		Texture* mRoughnessMap;
	};

	struct MaterialStore
	{
		std::string albedoMap;
		std::string alphaMap;
		std::string aoMap;
		std::string emissionMap;
		std::string metallicMap;
		std::string normalMap;
		std::string roughnessMap;

		//some materials only use a diffuse color
		glm::vec4 diffuseColor;

		MaterialType type = MaterialType::Pbr;
		RenderState state;

		static void test();
	};

	nex::BinStream& operator>>(nex::BinStream& in, MaterialStore& store);
	nex::BinStream& operator<<(nex::BinStream& out, const MaterialStore& store);
}