#include <nex/opengl/shader/SkyBoxShaderGL.hpp>
#include <nex/opengl/mesh/MeshGL.hpp>

using namespace glm;

SkyBoxShaderGL::SkyBoxShaderGL() : ShaderConfigGL(), skyTexture(nullptr)
{
	attributes.create(ShaderAttributeType::MAT4, nullptr, "projection");
	attributes.create(ShaderAttributeType::MAT4, nullptr, "view");
	attributes.create(ShaderAttributeType::MAT4, &transform, "transform", true);
	attributes.create(ShaderAttributeType::CUBE_MAP, nullptr, "skybox");
}

SkyBoxShaderGL::~SkyBoxShaderGL(){}

void SkyBoxShaderGL::afterDrawing(const MeshGL& mesh)
{
	glDepthFunc(GL_LESS); // The Type Of Depth Testing To Do
	glDepthMask(GL_TRUE);
}

void SkyBoxShaderGL::beforeDrawing(const MeshGL& mesh)
{
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
}

void SkyBoxShaderGL::setSkyTexture(CubeMapGL* sky)
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

PanoramaSkyBoxShaderGL::PanoramaSkyBoxShaderGL() : ShaderConfigGL(), 
skyTexture(nullptr)
{
	attributes.create(ShaderAttributeType::MAT4, nullptr, "projection");
	attributes.create(ShaderAttributeType::MAT4, nullptr, "view");
	//attributes.create(ShaderAttributeType::MAT4, &transform, "transform", true);
	attributes.create(ShaderAttributeType::TEXTURE2D, nullptr, "panorama");
}

PanoramaSkyBoxShaderGL::~PanoramaSkyBoxShaderGL(){}

void PanoramaSkyBoxShaderGL::afterDrawing(const MeshGL& mesh)
{
	glDepthFunc(GL_LESS); // The Type Of Depth Testing To Do
	glDepthMask(GL_TRUE);
}

void PanoramaSkyBoxShaderGL::beforeDrawing(const MeshGL& mesh)
{
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
}

void PanoramaSkyBoxShaderGL::setSkyTexture(TextureGL* tex)
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

EquirectangularSkyBoxShaderGL::EquirectangularSkyBoxShaderGL() : ShaderConfigGL(),
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

void EquirectangularSkyBoxShaderGL::afterDrawing(const MeshGL& mesh)
{
	glDepthFunc(GL_LESS); // The Type Of Depth Testing To Do
	glDepthMask(GL_TRUE);
}

void EquirectangularSkyBoxShaderGL::beforeDrawing(const MeshGL& mesh)
{
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
}

void EquirectangularSkyBoxShaderGL::setSkyTexture(TextureGL * tex)
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

	attributes.setData("projection", &projection);
	attributes.setData("view", &view);
}