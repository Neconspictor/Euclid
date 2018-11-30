#pragma once
#include <string>
#include <glad/glad.h>
#include <nex/exception/ShaderInitException.hpp>
#include <nex/common/Log.hpp>
#include <nex/shader_generator/ShaderSourceFileGenerator.hpp>
#include "nex/shader/Shader.hpp"

namespace nex
{

	// UniformLocation* will be reinterpreted as GLint
	// Thus we don't need any storage 
	class UniformLocation {};

	enum class ShaderStageGL
	{
		COMPUTE = GL_COMPUTE_SHADER,
		FRAGMENT = GL_FRAGMENT_SHADER,
		GEOMETRY = GL_GEOMETRY_SHADER,
		TESSELATION_CONTROL = GL_TESS_CONTROL_SHADER,
		TESSELATION_EVALUATION = GL_TESS_EVALUATION_SHADER,
		VERTEX = GL_VERTEX_SHADER,
	};


	ShaderStageGL translate(nex::ShaderStageType stage);
	nex::ShaderStageType translate(ShaderStageGL stage);


	/**
	 * Represents a shader program for an OpenGL renderer.
	 */
	class ShaderProgramGL : public ShaderProgram
	{
	public:

		ShaderProgramGL(const ShaderProgramGL& other) = delete;
		ShaderProgramGL& operator=(const ShaderProgramGL& other) = delete;

		GLuint getProgramID() const;

		static GLuint loadShaders(const std::string& vertexFile, const std::string& fragmentFile,
			const std::string& geometryShaderFile = "");

	protected:

		friend ShaderProgram;

		ShaderProgramGL(GLuint program);
		~ShaderProgramGL();


		static std::string adjustLineNumbers(char* message, const ShaderStageDesc& desc);
		static GLuint compileShaderStage(unsigned int type, const ShaderStageDesc& desc);
		static GLuint createShaderProgram(const ShaderStageDesc& vertexShader, const ShaderStageDesc& fragmentShader, const ShaderStageDesc* geometryShader = nullptr);

		/**
		 * @param shaderSourceFile The source file for that an unfolded version should be written for.
		 * @throws ShaderInitException - if an IO error occurs
		 */
		static void writeUnfoldedShaderContentToFile(const std::string& shaderSourceFile, const std::vector<char>& sourceCode);
		
		
		
		GLuint programID;
	};
}

std::ostream& operator<<(std::ostream& os, nex::ShaderStageGL type);