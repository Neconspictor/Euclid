#ifndef ENGINE_SHADER_OPENGL_PLAYGROUND_SHADERGL_HPP
#define ENGINE_SHADER_OPENGL_PLAYGROUND_SHADERGL_HPP
#include <shader/PlaygroundShader.hpp>
#include <shader/opengl/ShaderGL.hpp>

class Model;

class PlaygroundShaderGL : public ShaderGL, public PlaygroundShader
{
public:
	/**
	* Creates a new playground shader program.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	PlaygroundShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	virtual ~PlaygroundShaderGL();
	void draw(Model const& model, glm::mat4 const& transform) override;
	bool loadingFailed() override;
	void release() override;
	void setTexture1(const std::string& textureName) override;
	void setTexture2(const std::string& textureName) override;
	void setTextureMixValue(float mixValue) override;
	void use() override;

protected:
	float mixValue;

private:
	GLuint texture, texture2;
};

#endif
