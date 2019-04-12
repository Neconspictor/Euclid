#pragma once
#include <nex/util/StringUtils.hpp>
#include <nex/util/Memory.hpp>
#include <ostream>
#include <nex/shader/ShaderType.hpp>
#include <nex/texture/Sampler.hpp>

//namespace glm { class mat3; class mat4; class vec2; class vec3; class vec4; }

namespace nex
{
	class Sampler;
	enum class InternFormat;
	enum class TextureAccess;
	class Pass;
	struct ResolvedShaderStageDesc;
	class Material;
	class Texture;
	//class ShaderSourceFileGenerator;
	using FilePath = const char*;

	enum class ShaderStageType
	{
		COMPUTE = 0, SHADER_STAGE_FIRST = COMPUTE,
		FRAGMENT,
		GEOMETRY,
		TESSELATION_CONTROL,
		TESSELATION_EVALUATION,
		VERTEX, SHADER_STAGE_LAST = VERTEX
	};

	struct TransformData
	{
		glm::mat4 const* projection = nullptr;
		glm::mat4 const* view = nullptr;
		glm::mat4 const* model = nullptr;
	};

	class ShaderStage
	{
	public:

		nex::ShaderStage& operator=(const nex::ShaderStage& other) = delete;
		ShaderStage(const nex::ShaderStage& other) = delete;

		virtual ~ShaderStage() = default;

		static nex::ShaderStage* compileShaderStage(const nex::ResolvedShaderStageDesc& desc);

		ShaderStageType getType() const
		{
			return type;
		}

	protected:
		ShaderStage(ShaderStageType type) : type(type){};

		ShaderStageType type;
	};


/**
 * Represents a shader program for an OpenGL renderer.
 */
	class Shader
	{
	public:

		class Impl;

		Shader(std::unique_ptr<Impl> impl);

		virtual ~Shader() = default;

		template<typename T>
		static std::string makeDefine(const char* str, T value);

		/**
		 * Binds this shader.
		 */
		void bind();

		UniformTex createTextureUniform(const char* name, UniformType type, unsigned bindingSlot);

		/**
		 * Provides the uniform location by name. The memory is managed by this class and mustn't be freed manually.
		 */
		UniformLocation getUniformLocation(const char* name) const;

		UniformLocation getUniformBufferLocation(const char* name) const;

		UniformLocation getShaderStorageBufferLocation(const char* name) const;

		static std::unique_ptr<Shader> create(const FilePath& vertexFile, const FilePath& fragmentFile,
			const FilePath& geometryShaderFile = "", const std::vector<std::string>& defines = {});

		static std::unique_ptr<Shader> createComputeShader(const FilePath& computeFile, const std::vector<std::string>& defines = {});

		static std::unique_ptr<Shader> create(const std::vector<Guard<ShaderStage>>& stages);


		void setBinding(UniformLocation locationID, unsigned int bindingSlot);

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 * @param accessType : Specifies how the image will be accessed. Notices, that access violations will result 
		 *                     in undefined behaviour.
		 * @param level : The mip map level of the texture to bind. Should be zero for textures not having mip maps.                    
		 * @param textureIsArray: Specifies if the texture is an array. If true the whole array is bound as one image and the layer
		 *                        parameter is ignored.
		 *                        If false, the layer parameter specifies the layer of the texture array. 
		 *                        Should be zero for non array textures.
		 * @param format: Specifies the format that the elements of the image will be treated as for the purposes of formatted stores.
		 */
		void setImageLayerOfTexture(UniformLocation locationID, const nex::Texture* data, unsigned int bindingSlot, 
			TextureAccess accessType, InternFormat format, unsigned level, bool textureIsArray, unsigned layer);


		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound 
		 */
		void setInt(UniformLocation locationID, int data);

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void setFloat(UniformLocation locationID, float data);


		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void setUInt(UniformLocation locationID, unsigned data);

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void setUVec2(UniformLocation locationID, const glm::uvec2& data);

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void setUVec3(UniformLocation locationID, const glm::uvec3& data);

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void setUVec4(UniformLocation locationID, const glm::uvec4& data);


		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void setVec2(UniformLocation locationID, const glm::vec2& data);

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void setVec3(UniformLocation locationID, const glm::vec3& data);

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void setVec4(UniformLocation locationID, const glm::vec4& data);

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void setMat2(UniformLocation locationID, const glm::mat2& data);

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void setMat3(UniformLocation locationID, const glm::mat3& data);

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void setMat4(UniformLocation locationID, const glm::mat4& data);

		/**
		 * Binds a texture and a sampler object to a texture binding point. 
		 * @param texture : The texture to bind
		 * @param sampler : The sampler to bind or nullptr if no sampler should be bound
		 * @param bindingSlot : The binding point
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 * 
		 */
		void setTexture(const nex::Texture* texture, const Sampler* sampler, unsigned int bindingSlot);

		
		/**
		 * Sets a name for this shader program useful when debugging this class.
		 */
		void setDebugName(const char* name);

		/**
		 * Unbinds this shader.
		 */
		void unbind();


	protected:

		friend Pass;
		std::unique_ptr<Impl> mImpl;

	private:
		Shader& operator=(const Shader& other) = delete;
		Shader(const Shader& other) = delete;
	};

	template <typename T>
	std::string Shader::makeDefine(const char* str, T value)
	{
		return std::string("#define ") + std::string(str) + std::string(" ") + std::to_string(value);
	}

	std::ostream& operator<<(std::ostream& os, nex::ShaderStageType stageType);
}