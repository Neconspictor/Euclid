#pragma once
#include <string>
#include <glad/glad.h>
#include <nex/exception/ShaderInitException.hpp>
#include <nex/common/Log.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/shader_generator/ShaderSourceFileGenerator.hpp>

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
	class ShaderProgramGL : public ShaderProgram
	{
	public:

		ShaderProgramGL(const ShaderProgramGL& other) = delete;
		ShaderProgramGL& operator=(const ShaderProgramGL& other) = delete;

		GLuint getProgramID() const;

		static GLuint loadShaders(const std::vector<UnresolvedShaderStageDesc>& stageDescs);

	protected:

		friend ShaderProgram;
		friend ShaderStage;

		ShaderProgramGL(GLuint program);
		~ShaderProgramGL();


		static std::string adjustLineNumbers(char* message, const ResolvedShaderStageDesc& desc);
		static GLuint compileShaderStage(const ResolvedShaderStageDesc& desc, ShaderStageType type);
		static GLuint createShaderProgram(const std::vector<Guard<ShaderStage>>& stages);

		/**
		 * @param shaderSourceFile The source file for that an unfolded version should be written for.
		 * @throws ShaderInitException - if an IO error occurs
		 */
		static void writeUnfoldedShaderContentToFile(const std::string& shaderSourceFile, const std::vector<char>& sourceCode);
		
		
		
		GLuint programID;
	};
}