#ifndef ENGINE_SHADER_OPENGL_PLAYGROUND_SHADERGL_HPP
#define ENGINE_SHADER_OPENGL_PLAYGROUND_SHADERGL_HPP
#include <shader/PlaygroundShader.hpp>
#include <shader/opengl/ShaderGL.hpp>

class PlaygroundShaderGL : public ShaderGL, public PlaygroundShader
{
public:
	PlaygroundShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	virtual ~PlaygroundShaderGL();
	void draw(Model const& model, glm::mat4 const& transform) override;
	bool loadingFailed() override;
	void release() override;
	void setTexture1(const std::string& textureName) override;
	void setTexture2(const std::string& textureName) override;
	void use() override;

private:
	GLuint texture, texture2;
};

#endif