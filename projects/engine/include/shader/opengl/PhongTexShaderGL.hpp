#pragma once
#include <shader/opengl/ShaderGL.hpp>
#include <shader/PhongTextureShader.hpp>
#include <texture/opengl/TextureGL.hpp>

class PhongTexShaderGL : public ShaderGL, public PhongTextureShader
{
public:
	/**
	* Creates a new phong shader program.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	PhongTexShaderGL(const std::string& vertexShaderFile, 
		const std::string& fragmentShaderFile, const std::string& vertexShaderFileInstanced);

	virtual ~PhongTexShaderGL();

	void drawInstanced(Mesh const& mesh, unsigned int amount) override;

	void draw(Mesh const& mesh) override;

	const glm::vec3& getLightColor() const override;

	const glm::vec3& getLightPosition() const override;

	void release() override;

	void setLightColor(glm::vec3 color) override;

	void setLightDirection(glm::vec3 direction) override;

	void setLightSpaceMatrix(glm::mat4 mat) override;

	void setPointLightPositions(glm::vec3* positions) override;

	void setPointLightRange(float range) override;

	void setPointLightShadowMap(CubeDepthMap* map) override;

	void setShadowMap(Texture* texture) override;

	void setSkyBox(CubeMap* sky) override;

	void setSpotLightDirection(glm::vec3 direction) override;

	void setVarianceShadowMap(Texture* texture) override;

	void setViewPosition(glm::vec3 position) override;

	void use() override;

protected:
	void initForDrawing(Mesh const& meshOriginal, GLuint programID);

private:

	void initLights(GLuint programID);

	glm::vec3 dirLightDirection;
	GLuint instancedShaderProgram;
	glm::vec3 lightColor;
	glm::mat4 lightSpaceMatrix;
	glm::vec3 pointLightPositions[4];
	float pointLightRange;
	CubeDepthMapGL* pointLightShadowMap;
	glm::vec3 spotLightDirection;
	TextureGL* shadowMap;
	CubeMapGL* skybox;
	glm::vec3 viewPosition;
	TextureGL* vsMap;
};