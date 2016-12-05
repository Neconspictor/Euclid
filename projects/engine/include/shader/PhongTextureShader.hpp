#ifndef ENGINE_SHADER_PHONG_TEXTURE_SHADER_HPP
#define ENGINE_SHADER_PHONG_TEXTURE_SHADER_HPP
#include <shader/shader.hpp>
#include <material/PhongTexMaterial.hpp>

class PhongTextureShader : public Shader
{
public:
	PhongTextureShader() {}
	virtual ~PhongTextureShader() {};

	virtual const glm::vec3& getLightColor() const = 0;
	virtual const glm::vec3& getLightPosition() const = 0;

	virtual void setLightColor(glm::vec3 color) = 0;
	virtual void setLightPosition(glm::vec3 position) = 0;
	virtual void setMaterial(const PhongTexMaterial& material) = 0;
	virtual void setViewPosition(glm::vec3 position) = 0;
};
#endif