#pragma once
#include <string>
#include <glad/glad.h>
#include <nex/exception/ShaderInitException.hpp>
#include <nex/common/Log.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/shader_generator/ShaderSourceFileGenerator.hpp>
#include "nex/opengl/CacheGL.hpp"

namespace nex
{
	enum ShaderStageTypeGL
	{
		COMPUTE = GL_COMPUTE_SHADER,
		FRAGMENT = GL_FRAGMENT_SHADER,
		GEOMETRY = GL_GEOMETRY_SHADER,
		TESSELATION_CONTROL = GL_TESS_CONTROL_SHADER,
		TESSELATION_EVALUATION = GL_TESS_EVALUATION_SHADER,
		VERTEX = GL_VERTEX_SHADER,
	};

	std::ostream& operator<<(std::ostream& os, nex::ShaderStageTypeGL type);
	ShaderStageTypeGL translate(nex::ShaderStageType stage);
	nex::ShaderStageType translate(ShaderStageTypeGL stage);

	class ShaderStageGL : public ShaderStage
	{
	public:
		ShaderStageGL(GLuint shaderStage, ShaderStageType type);
		virtual ~ShaderStageGL();

		GLuint getID() const;

	private:
		GLuint mShaderStage;
	};


	/**
	 * Represents a shader program for an OpenGL renderer.
	 */
	class Shader::Impl
	{
	public:
		
		Impl(GLuint program);
		Impl() = delete;
		Impl(const Impl& other) = delete;
		Impl(Impl&& other) noexcept;
		Impl& operator=(const Impl& other) = delete;
		Impl& operator=(Impl&& other) noexcept;
		~Impl();
		

		void bind();

		GLuint getProgramID() const;
		nex::UniformLocation getShaderStorageBufferLocation(const char* name) const;
		nex::UniformLocation getUniformBufferLocation(const char* name) const;
		nex::UniformLocation getUniformLocation(const char* name) const;


		static GLuint loadShaders(const std::vector<UnresolvedShaderStageDesc>& stageDescs);

		void setBinding(UniformLocation locationID, unsigned bindingSlot);

		void setDebugName(const char* name);

		void setImageLayerOfTexture(UniformLocation locationID, const nex::Texture* data, unsigned int bindingSlot,
			TextureAccess accessType, InternFormat format, unsigned level, bool textureIsArray, unsigned layer);


		void setFloat(UniformLocation locationID, float data);

		void setMat2(UniformLocation locationID, const glm::mat2& data);
		void setMat3(UniformLocation locationID, const glm::mat3& data);
		void setMat4(UniformLocation locationID, const glm::mat4& data);

		void setInt(UniformLocation locationID, int data);

		void setTexture(const Texture* texture, const Sampler* sampler, unsigned bindingSlot);

		void setUInt(UniformLocation locationID, unsigned data);
		void setUVec2(UniformLocation locationID, const glm::uvec2& data);
		void setUVec3(UniformLocation locationID, const glm::uvec3& data);
		void setUVec4(UniformLocation locationID, const glm::uvec4& data);

		void setVec2(UniformLocation locationID, const glm::vec2& data);
		void setVec3(UniformLocation locationID, const glm::vec3& data);
		void setVec4(UniformLocation locationID, const glm::vec4& data);

		void unbind();



	protected:

		friend Shader;
		friend ShaderStage;

		static std::string adjustLineNumbers(char* message, const ResolvedShaderStageDesc& desc);
		static GLuint compileShaderStage(const ResolvedShaderStageDesc& desc, ShaderStageType type);
		static GLuint createShaderProgram(const std::vector<Guard<ShaderStage>>& stages);

		/**
		 * @param shaderSourceFile The source file for that an unfolded version should be written for.
		 * @throws ShaderInitException - if an IO error occurs
		 */
		static void writeUnfoldedShaderContentToFile(const std::string& shaderSourceFile, const std::vector<char>& sourceCode);

		std::string mDebugName;
		GLuint mProgramID;
		ShaderCacheGL mCache;
	};
}
