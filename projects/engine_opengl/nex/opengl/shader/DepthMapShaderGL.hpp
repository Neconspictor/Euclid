#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/opengl/texture/TextureGL.hpp>

class CubeDepthMapShaderGL : public ShaderGL
{
public:
	CubeDepthMapShaderGL();

	virtual ~CubeDepthMapShaderGL();
	
	void useCubeDepthMap(const CubeMapGL* map);

	void setLightPos(const glm::vec3& pos);

	void setRange(float range);

	void setModelMatrix(const glm::mat4& model);
	void setMVP(const glm::mat4& trafo);

private:
	UniformTex mCubeMap;
	Uniform mLightPos;
	Uniform mRange;
	Uniform mModel;
	Uniform mTransform;
};

class DepthMapShaderGL : public ShaderGL
{
public:
	DepthMapShaderGL();

	virtual ~DepthMapShaderGL() = default;

	void useDepthMapTexture(const TextureGL* texture);

	void setMVP(const glm::mat4& trafo);

private:
	UniformTex mDephTexture;
	Uniform mTransform;
};

class VarianceDepthMapShaderGL : public ShaderGL
{
public:
	VarianceDepthMapShaderGL();

	virtual ~VarianceDepthMapShaderGL() = default;

	void useVDepthMapTexture(const TextureGL* texture);

	void setMVP(const glm::mat4& trafo);

private:
	UniformTex mDephTexture;
	Uniform mTransform;
};