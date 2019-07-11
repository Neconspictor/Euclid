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

	/**
	 * A FilePath specifies a relative file path to a shader. The root directory thereby is the global shader directory which can be configured 
	 * by the FileSystem of the  ShaderSourceFileGenerator Singleton.
	 */
	using ShaderFilePath = const char*;

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

		/**
		 * Creates a new ShaderStage object. The object has to be freed by the caller.
		 * 
		 * @throws ShaderInitException: If the shader stage couldn't be compiled.
		 */
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

		Shader(const Shader&) = delete;
		Shader(Shader&&) = default;
		Shader& operator=(const Shader&) = delete;
		Shader& operator=(Shader&&)  = default;
		
		//Has to be default implemented by render implementation
		~Shader();
		
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
		int getUniformBufferBindingPoint(const char* name) const;

		UniformLocation getShaderStorageBufferLocation(const char* name) const;

		/**
		 * Creates a new shader intended to be used for producing color images. Each specified file plays a different role:
		 * @param vertexFile : Specifies the vertex shader. Is required and mustn't be nullptr
		 * @param fragmentFile: Specifies the fragment shader. Is required and mustn't be nullptr
		 * @param tesselationControlShaderFile: Specifies an optional tesselation control shader. If a tesselation evaluation shader is specified, 
		 *	this parameter mustn't be nullptr!
		 * @param tesselationEvaluationShader: Specifies an optional tesselation evaluation shader. If a tesselation control shader is specified, 
		 *	this parameter mustn't be nullptr!
		 * @param geometryShaderFile: Specifies an optional geometry shader. 
		 * @param defines: An optional list of define macros that can be used to configure the shader stages.
		 * 
		 * @throws: ShaderInitException: If an error occurs.
		 */
		static std::unique_ptr<Shader> create(
			const ShaderFilePath& vertexFile, 
			const ShaderFilePath& fragmentFile,
			const ShaderFilePath& tesselationControlShaderFile = nullptr,
			const ShaderFilePath& tesselationEvaluationShader = nullptr,
			const ShaderFilePath& geometryShaderFile = nullptr, 
			const std::vector<std::string>& defines = {});

		/**
		 * Creates a compute shader from file.
		 * @param computeFile: The compute shader file.
		 * @param defines: An optional list of define macros that can be used to configure the compute shader.
		 * @throws: ShaderInitException: If an error occurs.
		 */
		static std::unique_ptr<Shader> createComputeShader(const ShaderFilePath& computeFile, const std::vector<std::string>& defines = {});

		/**
		 * Creates a shader from a list of shader stages.
		 */
		static std::unique_ptr<Shader> create(const std::vector<Guard<ShaderStage>>& stages);

		/**
		 * Checks if the shader is currently bound
		 */
		bool isBound()const;


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
	};

	template <typename T>
	std::string Shader::makeDefine(const char* str, T value)
	{
		return std::string("#define ") + std::string(str) + std::string(" ") + std::to_string(value);
	}

	std::ostream& operator<<(std::ostream& os, nex::ShaderStageType stageType);
}