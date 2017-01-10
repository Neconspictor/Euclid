#include <antialiasing/opengl/SMAA_GL.hpp>
#include <texture/opengl/TextureManagerGL.hpp>
#include <gli/gli.hpp>
#include <SOIL2/SOIL2.h>
#include <shader/opengl/ShaderGL.hpp>


using namespace std;

SMAA_GL::SMAA_GL(RendererOpenGL* renderer) : SMAA(), renderer(renderer), areaTex(nullptr), searchTex(nullptr)
{
}

SMAA_GL::~SMAA_GL()
{
	renderer->destroyRenderTarget(&edgesTex);
	renderer->destroyRenderTarget(&blendTex);
}

void SMAA_GL::antialiase(RenderTarget* renderTarget)
{
}

void SMAA_GL::init()
{
	Renderer3D::Viewport viewPort = renderer->getViewport();
	int& width = viewPort.width;
	int& height = viewPort.height;

	edgesTex = renderer->createRenderTarget(GL_RGBA8, width, height, 1, GL_DEPTH_STENCIL);
	blendTex = renderer->createRenderTarget(GL_RGBA8, width, height, 1, GL_DEPTH_STENCIL);


	string areaTexfileName = TextureManagerGL::get()->getFullFilePath("_intern/smaa/AreaTexDX10.dds");
	string searchTexfileName = TextureManagerGL::get()->getFullFilePath("_intern/smaa/SearchTex.dds"); //SearchTex

	
	GLuint textureID = SOIL_load_OGL_texture(areaTexfileName.c_str(), SOIL_LOAD_AUTO, 0, SOIL_FLAG_INVERT_Y);
	if (textureID == GL_FALSE)
	{
		stringstream ss;
		ss << "SMAA_GL::init(): Couldn't load area texture: " << areaTexfileName;
		throw runtime_error(ss.str());
	}

	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	areaTex = TextureManagerGL::get()->createTextureGL("_intern/smaa/AreaTexDX10.dds", textureID);

	/*textureID = SOIL_load_OGL_texture(searchTexfileName.c_str(), SOIL_LOAD_AUTO, 0, SOIL_FLAG_INVERT_Y);
	if (textureID == GL_FALSE)
	{
		stringstream ss;
		ss << "SMAA_GL::init(): Couldn't load search texture: " << searchTexfileName;
		throw runtime_error(ss.str());
	}*/

	gli::texture Texture = gli::load(searchTexfileName.c_str());
	if (Texture.empty())
	{
		stringstream ss;
		ss << "SMAA_GL::init(): Couldn't load search texture: " << searchTexfileName;
		throw runtime_error(ss.str());
	}

	gli::gl GL(gli::gl::PROFILE_GL32);
	gli::gl::format const Format = GL.translate(Texture.format(), Texture.swizzles());
	GLenum Target = GL.translate(Texture.target());
	assert(Target == GL_TEXTURE_2D);

	textureID = 0;
	glGenTextures(1, &textureID);
	glBindTexture(Target, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	searchTex = TextureManagerGL::get()->createTextureGL("_intern/smaa/SearchTex.dds", textureID);

	/*GLuint TextureName = 0;
	glGenTextures(1, &TextureName);
	glBindTexture(Target, TextureName);
	glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));
	glTexParameteriv(Target, GL_TEXTURE_SWIZZLE_RGBA, &Format.Swizzles[0]);

	glm::tvec3<GLsizei> Extent(Texture.extent(0));

	glTexStorage2D(Target, static_cast<GLint>(Texture.levels()), Format.Internal, Extent.x, Extent.y);
	for (std::size_t Level = 0; Level < Texture.levels(); ++Level)
	{
		glm::tvec3<GLsizei> Extent(Texture.extent(Level));
		glCompressedTexSubImage2D(
			Target, static_cast<GLint>(Level), 0, 0, Extent.x, Extent.y,
			Format.Internal, static_cast<GLsizei>(Texture.size(Level)), Texture.data(0, 0, Level));
	}*/

	//GLuint testProgram = ShaderGL::loadShaders("playground_vs.glsl", "playground_fs.glsl");
}

void SMAA_GL::reset()
{
	renderer->clearFrameBuffer(blendTex.frameBuffer, { 0,0,0,1 }, 1.0f, 0);
	renderer->clearFrameBuffer(edgesTex.frameBuffer, { 0,0,0,1 }, 1.0f, 0);
}

void SMAA_GL::updateBuffers()
{
	renderer->destroyRenderTarget(&edgesTex);
	renderer->destroyRenderTarget(&blendTex);

	Renderer3D::Viewport viewPort = renderer->getViewport();
	int& width = viewPort.width;
	int& height = viewPort.height;

	edgesTex = renderer->createRenderTarget(GL_RGBA8, width, height, 1, GL_DEPTH_STENCIL);
	edgesTex = renderer->createRenderTarget(GL_RGBA8, width, height, 1, GL_DEPTH_STENCIL);
}