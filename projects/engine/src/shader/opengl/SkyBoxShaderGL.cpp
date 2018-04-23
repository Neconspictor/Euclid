#include <shader/opengl/SkyBoxShaderGL.hpp>
#include <mesh/opengl/MeshGL.hpp>

using namespace glm;

SkyBoxShaderGL::SkyBoxShaderGL() : SkyBoxShader(), ShaderConfigGL(), skyTexture(nullptr)
{
	attributes.create(ShaderAttributeType::MAT4, nullptr, "projection");
	attributes.create(ShaderAttributeType::MAT4, nullptr, "view");
	attributes.create(ShaderAttributeType::MAT4, &transform, "transform", true);
	attributes.create(ShaderAttributeType::CUBE_MAP, nullptr, "skybox");
}

SkyBoxShaderGL::~SkyBoxShaderGL(){}

void SkyBoxShaderGL::afterDrawing()
{
	glDepthFunc(GL_LESS); // The Type Of Depth Testing To Do
	glDepthMask(GL_TRUE);
}

void SkyBoxShaderGL::beforeDrawing()
{
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
}

void SkyBoxShaderGL::setSkyTexture(CubeMap* sky)
{
	skyTexture = dynamic_cast<CubeMapGL*>(sky);
	assert(skyTexture != nullptr);
	attributes.setData("skybox", skyTexture);
}

void SkyBoxShaderGL::update(const MeshGL& mesh, const TransformData& data)
{
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;
	transform = projection * view * model;
	attributes.setData("projection", &projection);
	attributes.setData("view", &view);
}

PanoramaSkyBoxShaderGL::PanoramaSkyBoxShaderGL() : PanoramaSkyBoxShader(), ShaderConfigGL(), 
skyTexture(nullptr)
{
	attributes.create(ShaderAttributeType::MAT4, nullptr, "projection");
	attributes.create(ShaderAttributeType::MAT4, nullptr, "view");
	//attributes.create(ShaderAttributeType::MAT4, &transform, "transform", true);
	attributes.create(ShaderAttributeType::TEXTURE2D, nullptr, "panorama");
}

PanoramaSkyBoxShaderGL::~PanoramaSkyBoxShaderGL(){}

void PanoramaSkyBoxShaderGL::afterDrawing()
{
	glDepthFunc(GL_LESS); // The Type Of Depth Testing To Do
	glDepthMask(GL_TRUE);
}

void PanoramaSkyBoxShaderGL::beforeDrawing()
{
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
}

void PanoramaSkyBoxShaderGL::setSkyTexture(Texture* tex)
{
	TextureGL* texGL = dynamic_cast<TextureGL*>(tex);
	assert(texGL != nullptr);
	skyTexture = texGL;
	attributes.setData("panorama", skyTexture);
}

void PanoramaSkyBoxShaderGL::update(const MeshGL& mesh, const TransformData& data)
{
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;
	//transform = projection * view * model;
	attributes.setData("projection", &projection);
	attributes.setData("view", &view);
}

EquirectangularSkyBoxShaderGL::EquirectangularSkyBoxShaderGL() : EquirectangularSkyBoxShader(), ShaderConfigGL(),
skyTexture(nullptr)
{
	attributes.create(ShaderAttributeType::MAT4, nullptr, "projection");
	attributes.create(ShaderAttributeType::MAT4, nullptr, "view");
	//attributes.create(ShaderAttributeType::MAT4, &transform, "transform", true);
	attributes.create(ShaderAttributeType::TEXTURE2D, nullptr, "equirectangularMap");
}

EquirectangularSkyBoxShaderGL::~EquirectangularSkyBoxShaderGL()
{
}

void EquirectangularSkyBoxShaderGL::afterDrawing()
{
	glDepthFunc(GL_LESS); // The Type Of Depth Testing To Do
	glDepthMask(GL_TRUE);
}

void EquirectangularSkyBoxShaderGL::beforeDrawing()
{
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
}

void EquirectangularSkyBoxShaderGL::setSkyTexture(Texture * tex)
{
	TextureGL* texGL = dynamic_cast<TextureGL*>(tex);
	assert(texGL != nullptr);
	skyTexture = texGL;
	attributes.setData("equirectangularMap", skyTexture);
}

void EquirectangularSkyBoxShaderGL::update(const MeshGL & mesh, const TransformData & data)
{
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;
	transform = projection * view * model;
	attributes.setData("projection", &projection);
	attributes.setData("view", &view);
}