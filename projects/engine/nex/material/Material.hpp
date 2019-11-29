#pragma once
#include "nex/util/StringUtils.hpp"
#include <nex/shader/ShaderType.hpp>
#include <nex/renderer/RenderTypes.hpp>
#include <string>

namespace nex
{
	class Shader;
	class BasePbrGeometryShader;
	class PbrGeometryShader;
	class PbrGeometryBonesShader;
	class Texture;
	class BinStream;
	class Sampler;
	class SimpleColorPass;

	enum class MaterialType
	{
		Pbr,
		SimpleColor,
		None
	};

	class Material
	{
	public:
		Material(Shader* shader);

		virtual ~Material();

		RenderState& getRenderState();
		const RenderState& getRenderState() const;

		Shader* getShader();
		Shader* getShader() const;

		void setShader(Shader* technique);

	protected:
		Shader* mShader;
		RenderState mRenderState;
	};

	/**
		* Maps material enumerations to a string representation.
		*/
	static const util::EnumString<MaterialType> materialEnumConversion[] = {
		{ nex::MaterialType::Pbr, "PBR" },
		{ nex::MaterialType::SimpleColor, "SIMPLE_COLOR" },
		{ nex::MaterialType::None, "NONE" }
	};

	std::ostream& operator<<(std::ostream& os, nex::MaterialType type);


	class PbrMaterial : public Material
	{
	public:

		PbrMaterial(BasePbrGeometryShader* shader);
		PbrMaterial(
			BasePbrGeometryShader* shader,
			Texture* albedoMap,
			Texture* aoMap,
			Texture* emissionMap,
			Texture* metallicMap,
			Texture* normalMap,
			Texture* roughnessMap);

		virtual ~PbrMaterial() = default;

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

	protected:
		Texture* mAlbedoMap;
		Texture* mAoMap;
		Texture* mEmissionMap;
		Texture* mMetallicMap;
		Texture* mNormalMap;
		Texture* mRoughnessMap;
	};

	class SimpleColorMaterial : public Material 
	{
	public:

		SimpleColorMaterial(SimpleColorPass* shader);

		virtual ~SimpleColorMaterial() = default;

		void setColor(const glm::vec4& color);

		const glm::vec4& getColor() const;

	private:
		glm::vec4 mColor;
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