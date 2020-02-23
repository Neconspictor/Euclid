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
	class ShaderProvider;

	enum class MaterialType
	{
		Pbr,
		SimpleColor,
		None
	};

	class Material
	{
	public:
		Material(std::shared_ptr<ShaderProvider> provider);

		virtual ~Material();

		RenderState& getRenderState();
		const RenderState& getRenderState() const;

		Shader* getShader();
		Shader* getShader() const;

		void setShaderProvider(std::shared_ptr<ShaderProvider> provider);

	protected:
		std::shared_ptr<ShaderProvider> mShaderProvider;
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

		PbrMaterial(std::shared_ptr<ShaderProvider>shader);
		PbrMaterial(
			std::shared_ptr<ShaderProvider> shader,
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

		SimpleColorMaterial(std::shared_ptr<ShaderProvider> provider);

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

		//Specifies if the material is constructed for a skinned mesh type
		bool isSkinned;

		RenderState state;

		static void test();
	};

	nex::BinStream& operator>>(nex::BinStream& in, MaterialStore& store);
	nex::BinStream& operator<<(nex::BinStream& out, const MaterialStore& store);
}