#pragma once
#include "nex/util/StringUtils.hpp"
#include <nex/shader/ShaderType.hpp>

namespace nex
{
	class ShaderProgram;
	class Texture;

	enum class MaterialType
	{
		Pbr,
		None
	};

	class Material
	{
	public:
		Material();

		virtual ~Material() = default;

		ShaderProgram* getProgram();

		void init(ShaderProgram* program);

		void set(UniformLocation loc, float value);
		void set(UniformLocation loc, int value);
		void set(UniformLocation loc, const glm::mat2& value);
		void set(UniformLocation loc, const glm::mat3& value);
		void set(UniformLocation loc, const glm::mat4& value);
		void set(UniformLocation loc, unsigned value);
		void set(UniformLocation loc, const glm::u32vec2& value);
		void set(UniformLocation loc, const glm::u32vec3& value);
		void set(UniformLocation loc, const glm::u32vec4& value);
		void set(UniformLocation loc, const glm::vec2& value);
		void set(UniformLocation loc, const glm::vec3& value);
		void set(UniformLocation loc, const glm::vec4& value);
		void set(unsigned bindingSlot, const Texture* texture);

		void setProgram(ShaderProgram* program);

		/**
		 * Transfers the set uniforms from RAM to the GPU for the shader program of this material.
		 */
		void upload() const;


	protected:
		template<typename T>
		using Map = std::unordered_map<UniformLocation, T>;

		using MapTexture = std::unordered_map<int, const Texture*>;

		MapTexture mTextures;
		Map<float> mFloats;
		Map<int> mInts;
		Map<glm::mat2> mMat2s;
		Map<glm::mat3> mMat3s;
		Map<glm::mat4> mMat4s;
		Map<unsigned> mUints;
		Map<glm::uvec2> mUVec2s;
		Map<glm::uvec3> mUVec3s;
		Map<glm::uvec4> mUVec4s;
		Map<glm::vec2> mVec2s;
		Map<glm::vec3> mVec3s;
		Map<glm::vec4> mVec4s;

		ShaderProgram* mProgram;
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

		PbrMaterial();
		PbrMaterial(Texture* albedoMap,
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

	};
}