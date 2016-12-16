#pragma once
#include <shader/PlaygroundShader.hpp>
#include <shader/opengl/ShaderGL.hpp>
#include <texture/opengl/TextureGL.hpp>

class Vob;

class PlaygroundShaderGL : public ShaderGL, public PlaygroundShader
{
public:
	/**
	* Creates a new playground shader program.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	PlaygroundShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	virtual ~PlaygroundShaderGL();
	void draw(Mesh const& mesh) override;
	void release() override;
	void setTexture1(const std::string& textureName) override;
	void setTexture2(const std::string& textureName) override;
	void setTextureMixValue(float mixValue) override;
	void use() override;

protected:
	float mixValue;

private:
	TextureGL* texture;
	TextureGL* texture2;
};