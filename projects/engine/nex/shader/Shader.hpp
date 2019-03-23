#pragma once
#include <nex/util/StringUtils.hpp>
#include <nex/util/Memory.hpp>
#include <ostream>

//namespace glm { class mat3; class mat4; class vec2; class vec3; class vec4; }

namespace nex
{
	enum class InternFormat;
	enum class TextureAccess;
	class Shader;
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

	enum class UniformType
	{
		CUBE_MAP,
		FLOAT,
		INT,
		MAT3,
		MAT4,
		TEXTURE2D,
		TEXTURE2D_ARRAY,
		UINT,
		UVEC2,
		UVEC3,
		UVEC4,
		VEC2,
		VEC3,
		VEC4
	};

	using UniformLocation = int;

	struct Uniform
	{
		UniformLocation location = -1;
		UniformType type = UniformType::INT;
	};
	

	struct UniformTex : public Uniform
	{
		unsigned int bindingSlot = 0;
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
	class ShaderProgram
	{
	public:

		virtual ~ShaderProgram() = default;

		template<typename T>
		static std::string makeDefine(const char* str, T value);

		/**
		 * Binds this shader program.
		 */
		void bind();

		/**
		 * Provides the uniform location by name. The memory is managed by this class and mustn't be freed manually.
		 */
		UniformLocation getUniformLocation(const char* name) const;

		UniformLocation getUniformBufferLocation(const char* name) const;

		UniformLocation getShaderStorageBufferLocation(const char* name) const;

		static std::unique_ptr<ShaderProgram> create(const FilePath& vertexFile, const FilePath& fragmentFile,
			const FilePath& geometryShaderFile = "", const std::vector<std::string>& defines = {});

		static std::unique_ptr<ShaderProgram> createComputeShader(const FilePath& computeFile, const std::vector<std::string>& defines = {});

		static std::unique_ptr<ShaderProgram> create(const std::vector<Guard<ShaderStage>>& stages);

		void release();

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
		void setMat3(UniformLocation locationID, const glm::mat3& data);

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void setMat4(UniformLocation locationID, const glm::mat4& data);

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void setTexture(UniformLocation locationID, const nex::Texture* data, unsigned int bindingSlot);

		
		/**
		 * Sets a name for this shader program useful when debugging this class.
		 */
		void setDebugName(const char* name);

		/**
		 * Unbinds this shader program.
		 */
		void unbind();


	protected:

		friend Shader;

		ShaderProgram(void* impl);;

		bool mIsBound;
		std::string mDebugName;

	public:
		void* mImpl;

	private:
		ShaderProgram& operator=(const ShaderProgram& other) = delete;
		ShaderProgram(const ShaderProgram& other) = delete;
	};

	template <typename T>
	std::string ShaderProgram::makeDefine(const char* str, T value)
	{
		return std::string("#define ") + std::string(str) + std::string(" ") + std::to_string(value);
	}


	class Shader
	{
	public:

		Shader(std::unique_ptr<ShaderProgram> program = nullptr);

		virtual ~Shader() = default;

		/**
		 * Binds this shader and the underlying shader program.
		 */
		void bind();

		ShaderProgram* getProgram();

		void setProgram(std::unique_ptr<ShaderProgram> program);

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

	protected:

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		std::unique_ptr<ShaderProgram> mProgram;
	};

	class TransformShader : public Shader
	{
	public:
		TransformShader(std::unique_ptr<ShaderProgram> program = nullptr);
		virtual ~TransformShader() = default;

		virtual void onTransformUpdate(const TransformData& data) = 0;
	};

	class ComputeShader : public Shader
	{
	public:
		ComputeShader(std::unique_ptr<ShaderProgram> program = nullptr);
		virtual ~ComputeShader() = default;

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